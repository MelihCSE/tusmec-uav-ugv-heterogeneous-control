#ifndef COORDINATOR_H
#define COORDINATOR_H

#include <stdbool.h>
#include "planner.h"
#include "vehicle.h"

#define MAX_VEHICLES 64

typedef enum 
{
    STATE_INIT = 0,
    STATE_UAV_TAKEOFF,
    STATE_AREA_SCAN,
    STATE_UGV_ASSIGN,
    STATE_MISSION_COMPLETE
} Mission_Status;

typedef struct 
{
    Vehicle_t* vehicles[MAX_VEHICLES];
    int vehicle_count;

    Mission_Status status;
    Rotation_t scan_points[MAX_WAYPOINTS];
    int total_scan_points;
    int current_wp_index;

    Rotation_t target_point;
    bool target_found;
} Coordinator_t;

void coordinator_init(Coordinator_t* coord);

int coordinator_add_vehicle(Coordinator_t* coord, Vehicle_t* vehicle);

int coordinator_step(Coordinator_t* coord);

#endif /* COORDINATOR_H */
