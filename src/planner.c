#include "planner.h"
#include <math.h>
#include <stddef.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define EARTH_RADIUS_M 6378137.0

// Converting the North(dy) and East(dx) deflection to GPS coordinate
static Rotation_t offset_gps(Rotation_t origin, double dx_m, double dy_m, double target_alt) {
    Rotation_t pt;
    double lat_rad = origin.latitude * (M_PI / 180.0);

    pt.latitude  = origin.latitude  + (dy_m / EARTH_RADIUS_M) * (180.0 / M_PI);
    pt.longitude = origin.longitude + (dx_m / (EARTH_RADIUS_M * cos(lat_rad))) * (180.0 / M_PI);
    pt.altitude  = target_alt;
    return pt;
}

// --- UN_COMPLICATED AREA SCAN ALGORITHM --- // 
int boustrophedon(Rotation_t center, double width_m, double height_m, double step_m,
                  Rotation_t *waypoints, int max_waypoints) {
    if (waypoints == NULL || max_waypoints <= 0 || step_m <= 0.0) {
        return 0;
    }

    int count = 0;
    int num_rows = (int)(height_m / step_m) + 1;

    // Position of the bottom-left corner of the area relative to the center, in meters
    double start_x = -width_m / 2.0;
    double end_x   =  width_m / 2.0;
    double start_y = -height_m / 2.0;

    for (int row = 0; row < num_rows; row++) {
        double current_y = start_y + (row * step_m);
        if (current_y > height_m / 2.0) {
            current_y = height_m / 2.0;
        }

        // Draw a zigzag pattern from left to right on even lines and from right to left on odd lines.
        double first_x  = (row % 2 == 0) ? start_x : end_x;
        double second_x = (row % 2 == 0) ? end_x   : start_x;

        if (count < max_waypoints) {
            waypoints[count++] = offset_gps(center, first_x, current_y, center.altitude);
        }
        if (count < max_waypoints) {
            waypoints[count++] = offset_gps(center, second_x, current_y, center.altitude);
        }
    }

    return count;
}

// GPS distance calculator
double gps_distance(Rotation_t p1, Rotation_t p2) {
    double lat1 = p1.latitude * (M_PI / 180.0);
    double lat2 = p2.latitude * (M_PI / 180.0);
    double dlat = (p2.latitude - p1.latitude) * (M_PI / 180.0);
    double dlon = (p2.longitude - p1.longitude) * (M_PI / 180.0);

    double a = sin(dlat / 2.0) * sin(dlat / 2.0) +
               cos(lat1) * cos(lat2) * sin(dlon / 2.0) * sin(dlon / 2.0);
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));

    return EARTH_RADIUS_M * c;
}
