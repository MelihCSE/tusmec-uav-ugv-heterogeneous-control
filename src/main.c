#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#include "coordinator.h"
#include "vehicle.h"
#include "net_tcp.h"

int main(void){
    // 1. Create vehicles
    Vehicle_t uav = UAV_create(1, 5762);
    Vehicle_t ugv = UGV_create(2, 5772);

    // 2. Provide connection and check connection control
    if(uav.sock_fd < 0){
        fprintf(stderr, "Error: Failed UAV TCP connection (port: %u).", uav.port);
        exit(EXIT_FAILURE);
    }
    if(ugv.sock_fd < 0){
        fprintf(stderr, "Error: Failed UGV TCP connection (port: %u).", ugv.port);
        tcp_close(uav.sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("UAV & UGV is connected succesfully!\n");

    // 3. Declare coordinator_t structure 
    Coordinator_t coord; 
    coordinator_init(&coord);
    if(coordinator_add_vehicle(&coord, &uav) < 0){
        fprintf(stderr, "Error: Failed adding vehicle: UAV.");
        tcp_close(uav.sock_fd);
        tcp_close(ugv.sock_fd);
        exit(EXIT_FAILURE);
    }
    if(coordinator_add_vehicle(&coord, &ugv) < 0){
        fprintf(stderr, "Error: Failed adding vehicle: UGV.");
        tcp_close(uav.sock_fd);
        tcp_close(ugv.sock_fd);
        exit(EXIT_FAILURE);
    }

    // 4. Main mission cycle
    while(true){
        int res = coordinator_step(&coord);

        if(res == 1){
            printf("Mission completed successfully!\n");
            break;
        }
        else if(res < 0){
            fprintf(stderr, "Error: Mission aborted due to an error.\n");
            break;
        }

        // Waiting for 100ms = 10Hz
        usleep(100000);
    }

    // 5. Close sockets 
    tcp_close(uav.sock_fd);
    tcp_close(ugv.sock_fd);

    return 0;
}