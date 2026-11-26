#include "vehicle.h"
#include "net_tcp.h"
#include "mavlink_helper.h"

#include <stdint.h>
#include <unistd.h>

// ArduCopter Flight Mode Constants
#define COPTER_MODE_GUIDED 4
#define COPTER_MODE_LOITER 5
#define COPTER_MODE_LAND   9

// Saving every UAV's MAVLink packet parser for theri IDs 
static MavlinkParser_t uav_parsers[16];

// VTable->update_telemetry
static void uav_update_telemetry(Vehicle_t* self){
    if(self == NULL) return;
    MavlinkParser_t* parser = &uav_parsers[self->mavlink_id % 16];
    mavlink_poll_telemetry(self, parser);
}

// VTable->take_off
static int uav_take_off(Vehicle_t* self, double altitude){
    if(self == NULL) return -1;

    // Set GUIDED Mode
    if(mavlink_send_set_mode(self->sock_fd, self->mavlink_id, COPTER_MODE_GUIDED) < 0){
        return -1;
    }

    usleep(200000);

    if(mavlink_send_arm(self->sock_fd, self->mavlink_id, true) < 0){
        return -1;
    }

    usleep(500000);

    return mavlink_send_takeoff(self->sock_fd, self->mavlink_id, altitude);
}

// VTable->landing
static int uav_landing(Vehicle_t* self){
    if(self == NULL) return -1;

    return mavlink_send_set_mode(self->sock_fd, self->mavlink_id, COPTER_MODE_LAND);
}

// VTable->guidance
static void uav_guidance(Vehicle_t* self, double lon, double lat, double alt){
    if(self == NULL) return;

    mavlink_send_guided_target(self->sock_fd, self->mavlink_id, lat, lon, alt);
}

// VTable->hold_vehicle
static void uav_hold_vehicle(Vehicle_t* self){
    if(self == NULL) return;

    mavlink_send_set_mode(self->sock_fd, self->mavlink_id, COPTER_MODE_LOITER);
}

Vehicle_t UAV_create(uint8_t id, uint16_t port){
    Vehicle_t uav;
    memset(&uav, 0, sizeof(Vehicle_t));

    snprintf(uav.name, VEHICLE_NAME_LENGTH, "UAV_%u", id);
    uav.mavlink_id = id;
    uav.port = port;

    uav.sock_fd = tcp_connect("127.0.0.1", port);
    if (uav.sock_fd >= 0) {
        // Request 10hz temeletry flow
        mavlink_request_data_stream(uav.sock_fd, uav.mavlink_id, 10);
    }

    // Connect VTable polymorfic functions to UAV 
    uav.VTable.update_telemetry = uav_update_telemetry;
    uav.VTable.take_off         = uav_take_off;
    uav.VTable.landing          = uav_landing;
    uav.VTable.guidance         = uav_guidance;
    uav.VTable.hold_vehicle     = uav_hold_vehicle;

    return uav;
}
