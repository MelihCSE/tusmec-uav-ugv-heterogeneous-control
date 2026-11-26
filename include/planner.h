#ifndef PLANNER_H
#define PLANNER_H

#define MAX_WAYPOINTS 128

typedef struct 
{
    double latitude;
    double longitude;
    double altitude;
} Rotation_t;

int boustrophedon(Rotation_t center, double width_m, double height_m, double step_m,
            Rotation_t* waypoints, int max_waypoints);

double gps_distance(Rotation_t p1, Rotation_t p2);

#endif /* PLANNER_H */