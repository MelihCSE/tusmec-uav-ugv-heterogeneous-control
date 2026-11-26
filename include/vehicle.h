#ifndef VEHICLE_H
#define VEHICLE_H

#include <stdbool.h>
#include <stdint.h>

#define VEHICLE_NAME_LENGTH 128

typedef struct Vehicle Vehicle_t;

typedef struct 
{
    double longitude;
    double latitude;    
    double altitude;
    uint8_t autopilot;
    bool isArmed;
    bool gps_flag;

} Telemetry_t;

typedef struct 
{
    void (*update_telemetry)(Vehicle_t *self);
    int (*take_off)(Vehicle_t *self, double altitude);
    int (*landing)(Vehicle_t *self);
    void (*guidance)(Vehicle_t *self, double longitude, double latitude, double altitude);
    void (*hold_vehicle)(Vehicle_t *self);

} VTable_t;

typedef struct Vehicle 
{
    char name[VEHICLE_NAME_LENGTH];
    int sock_fd;
    uint16_t port;
    uint8_t mavlink_id;
    VTable_t VTable;
    Telemetry_t telemetry;

} Vehicle_t;

Vehicle_t UAV_create(uint8_t id, uint16_t port);
Vehicle_t UGV_create(uint8_t id, uint16_t port);


#endif /* VEHICLE_H */