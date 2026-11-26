#include "coordinator.h"
#include <stdio.h>
#include <string.h>

#define SCAN_ALTITUDE_M        15.0
#define SCAN_AREA_WIDTH_M      40.0
#define SCAN_AREA_HEIGHT_M     40.0
#define SCAN_STEP_M            10.0

#define WAYPOINT_TOLERANCE_M    2.0
#define DETECTION_RADIUS_M      6.0

void coordinator_init(Coordinator_t *coord) {
    if (coord == NULL) return;
    memset(coord, 0, sizeof(Coordinator_t));
    coord->status = STATE_INIT;
}

int coordinator_add_vehicle(Coordinator_t *coord, Vehicle_t *vehicle) {
    if (coord == NULL || vehicle == NULL || coord->vehicle_count >= MAX_VEHICLES) {
        return -1;
    }
    coord->vehicles[coord->vehicle_count++] = vehicle;
    return 0;
}

// Helper: Finds the vehicles by searching for their names.
static Vehicle_t* get_vehicle_by_prefix(Coordinator_t *coord, const char *prefix) {
    for (int i = 0; i < coord->vehicle_count; i++) {
        if (strncmp(coord->vehicles[i]->name, prefix, strlen(prefix)) == 0) {
            return coord->vehicles[i];
        }
    }
    return NULL;
}

int coordinator_step(Coordinator_t *coord) {
    if (coord == NULL || coord->vehicle_count < 2) {
        return -1;
    }

    // Update the telemetry of all vehicles via the polymorphic VTable interface 
    for (int i = 0; i < coord->vehicle_count; i++) {
        Vehicle_t *v = coord->vehicles[i];
        if (v->VTable.update_telemetry != NULL) {
            v->VTable.update_telemetry(v);
        }
    }

    Vehicle_t *uav = get_vehicle_by_prefix(coord, "UAV");
    Vehicle_t *ugv = get_vehicle_by_prefix(coord, "UGV");
    if (uav == NULL || ugv == NULL) return -1;

    Rotation_t uav_pos = {uav->telemetry.latitude, uav->telemetry.longitude, uav->telemetry.altitude};
    Rotation_t ugv_pos = {ugv->telemetry.latitude, ugv->telemetry.longitude, 0.0};

    switch (coord->status) {
        case STATE_INIT: {
            // Wait until receive valid GPS locations from both vehicles.
            if (!uav->telemetry.gps_flag || !ugv->telemetry.gps_flag) {
                return 0;
            }

            // Generate a Boustrophedon scanning route
            Rotation_t center = {uav_pos.latitude, uav_pos.longitude, SCAN_ALTITUDE_M};
            coord->total_scan_points = boustrophedon(
                center, SCAN_AREA_WIDTH_M, SCAN_AREA_HEIGHT_M, SCAN_STEP_M,
                coord->scan_points, MAX_WAYPOINTS
            );

            // Designate a virtual target point (3rd scanning point).
            int target_wp = (coord->total_scan_points > 3) ? 3 : 0;
            coord->target_point = coord->scan_points[target_wp];
            coord->target_point.altitude = 0.0;

            printf("[COORD] GPS locked. %d scan points generated. Take-Off begins...\n",
                   coord->total_scan_points);

            // Take-off UAV, setting UGV
            uav->VTable.take_off(uav, SCAN_ALTITUDE_M);
            ugv->VTable.take_off(ugv, 0.0);

            coord->status = STATE_UAV_TAKEOFF;
            break;
        }

        case STATE_UAV_TAKEOFF: {
            // Once it reaches the target altitude, UAV will begin area scanning
            if (uav->telemetry.altitude >= (SCAN_ALTITUDE_M - 1.0)) {
                printf("[COORD] UAV reached %.1f m altitude. Field scanning begins!\n",
                       uav->telemetry.altitude);
                coord->current_wp_index = 0;
                coord->status = STATE_AREA_SCAN;
            }
            break;
        }

        case STATE_AREA_SCAN:
        case STATE_UGV_ASSIGN: {
            // --- UAV Mission: Keep scan Boustrophedon ---
            if (coord->current_wp_index < coord->total_scan_points) {
                Rotation_t wp = coord->scan_points[coord->current_wp_index];
                uav->VTable.guidance(uav, wp.longitude, wp.latitude, wp.altitude);

                if (gps_distance(uav_pos, wp) < WAYPOINT_TOLERANCE_M) {
                    printf("[COORD] UAV scan point %d/%d is completed.\n",
                           coord->current_wp_index + 1, coord->total_scan_points);
                    coord->current_wp_index++;
                }
            }

            // --- Target Detection (Check if its not found yet) ---
            if (!coord->target_found &&
                gps_distance(uav_pos, coord->target_point) < DETECTION_RADIUS_M) {
                coord->target_found = true;
                coord->status = STATE_UGV_ASSIGN;
                printf("[COORD] TARGET DETECTED! (%.6f, %.6f) -> UGV IS BEING ASSIGNED!\n",
                       coord->target_point.latitude, coord->target_point.longitude);
            }

            // --- UGV Mission: If target found then move towards to target ---
            if (coord->status == STATE_UGV_ASSIGN) {
                ugv->VTable.guidance(ugv, coord->target_point.longitude,
                                     coord->target_point.latitude, 0.0);

                double dist_to_target = gps_distance(ugv_pos, coord->target_point);
                if (dist_to_target < WAYPOINT_TOLERANCE_M) {
                    printf("[COORD] UGV reach target. (distance: %.2f m). MISSION COMPLETE!\n",
                           dist_to_target);
                    coord->status = STATE_MISSION_COMPLETE;
                }
            }
            break;
        }

        case STATE_MISSION_COMPLETE: {
            ugv->VTable.hold_vehicle(ugv);
            uav->VTable.landing(uav);
            return 1; // Tell the main cycle that missin is over
        }
    }

    return 0; // Mission keeps going
}
