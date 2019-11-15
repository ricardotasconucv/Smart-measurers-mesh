/* Mesh Internal Communication Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mesh.h"
#include "esp_mesh_internal.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

/*******************************************************
 *                Macros
 *******************************************************/

/*******************************************************
 *                Constants
 *******************************************************/
#define LED_PAPA 2
#define PULSADOR_PIN 23
#define RX_SIZE          (1500)
#define TX_SIZE          (1460)
#define CONFIG_MESH_ROUTER_SSID "EstelitaTwo"
#define CONFIG_MESH_CHANNEL 0
#define CONFIG_MESH_MAX_LAYER 10
#define CONFIG_MESH_ROUTER_PASSWD "pancho$5gomita"
#define CONFIG_MESH_AP_AUTHMODE WIFI_AUTH_WPA2_PSK
#define CONFIG_MESH_AP_CONNECTIONS 2
#define CONFIG_MESH_AP_PASSWD "-Capacho10"
#define CONFIG_MESH_ROUTE_TABLE_SIZE 50
#define ESP_INTR_FLAG_DEFAULT 0

/*******************************************************
 *                Variable Definitions
 *******************************************************/
static const char *MESH_TAG = "mesh_main";
static const uint8_t MESH_ID[6] = { 0x77, 0x77, 0x77, 0x77, 0x77, 0x77};
static uint8_t tx_buf[TX_SIZE] = { 0, };
static uint8_t rx_buf[RX_SIZE] = { 0, };
static bool is_mesh_connected = false;
static mesh_addr_t mesh_parent_addr;
static int mesh_layer = -1;
uint32_t contador=0;
SemaphoreHandle_t semaforo = NULL;

/*******************************************************
 *                Function Declarations
 *******************************************************/

/*******************************************************
 *                Function Definitions
 *******************************************************/
void IRAM_ATTR pulsador_isr_handler(void* arg) {
    // da el semáforo para que quede libre para la tarea pulsador
   xSemaphoreGiveFromISR(semaforo, NULL);
}

void esp_mesh_p2p_rx_main(void *arg)
{
    union auxi{
        uint8_t aux[4];
        uint32_t valor;
    }cuanto;
    mesh_addr_t from;
    mesh_data_t data;
    int flag = 0;
    data.data = rx_buf;
    data.size = RX_SIZE;
    mesh_addr_t route_table[CONFIG_MESH_ROUTE_TABLE_SIZE];
    int route_table_size=0;
    enum mac{
        caso_1=0xC8,
        caso_2=0x44,
        caso_3=0x74,
        caso_4=0x40
    }mac1;


    while (1) {
       data.size = RX_SIZE;
       esp_mesh_recv(&from, &data, portMAX_DELAY, &flag, NULL, 0);

        //esp_mesh_get_routing_table((mesh_addr_t *) &route_table,
         //   CONFIG_MESH_ROUTE_TABLE_SIZE * 6, &route_table_size);
       mac1=from.addr[5];
       switch(mac1){

           case caso_1:
           for(uint16_t f=0;f<data.size;f++){
               cuanto.aux[f]=data.data[f];
            }
            printf("MAC: "MACSTR" Contador= %d\n",MAC2STR(from.addr),cuanto.valor);
            mac1=0;
           break;

           case caso_2:
           for(uint16_t f=0;f<data.size;f++){
               cuanto.aux[f]=data.data[f];
            }
            printf("MAC: "MACSTR" Contador= %d\n",MAC2STR(from.addr),cuanto.valor);
            mac1=0;
           break;
           case caso_3:
           for(uint16_t f=0;f<data.size;f++){
               cuanto.aux[f]=data.data[f];
            }
            printf("MAC: "MACSTR" Contador= %d\n",MAC2STR(from.addr),cuanto.valor);
            mac1=0;
           break;
           case caso_4:
           for(uint16_t f=0;f<data.size;f++){
               cuanto.aux[f]=data.data[f];
            }
            printf("MAC: "MACSTR" Contador= %d\n",MAC2STR(from.addr),cuanto.valor);
            mac1=0;
           break;

           default:
            //printf("En espera\n");
           break;


        }
       vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelete(NULL);
}

void botonCounter(void *Pa){
    mesh_addr_t dir;
    uint8_t aux[6]={0xA4,0xCF,0x12,0x75,0xB8,0x10};
     memcpy(dir.addr,aux,6);
    mesh_data_t datos;
    datos.size=sizeof(contador);
    datos.proto=MESH_PROTO_BIN;
    datos.data=&contador;
    while(1){

         if(xSemaphoreTake(semaforo,portMAX_DELAY) == pdTRUE)
              {
               esp_mesh_send(NULL,&datos,MESH_DATA_P2P,NULL,0);
               printf("interrupcion\n");
               printf("Contador = %d\n",contador);
               contador++;
                }
         vTaskDelay(pdMS_TO_TICKS(10));
             }


}



void mesh_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    mesh_addr_t id = {0,};
    static uint8_t last_layer = 0;

    switch (event_id) {
    case MESH_EVENT_STARTED: {
        esp_mesh_get_id(&id);
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_MESH_STARTED>ID:"MACSTR"", MAC2STR(id.addr));
        is_mesh_connected = false;
        mesh_layer = esp_mesh_get_layer();
    }
    break;
    case MESH_EVENT_STOPPED: {
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_STOPPED>");
        is_mesh_connected = false;
        mesh_layer = esp_mesh_get_layer();
    }
    break;
    case MESH_EVENT_CHILD_CONNECTED: {
        mesh_event_child_connected_t *child_connected = (mesh_event_child_connected_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_CHILD_CONNECTED>aid:%d, "MACSTR"",
                 child_connected->aid,
                 MAC2STR(child_connected->mac));
    }
    break;
    case MESH_EVENT_CHILD_DISCONNECTED: {
        mesh_event_child_disconnected_t *child_disconnected = (mesh_event_child_disconnected_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_CHILD_DISCONNECTED>aid:%d, "MACSTR"",
                 child_disconnected->aid,
                 MAC2STR(child_disconnected->mac));
    }
    break;
    case MESH_EVENT_ROUTING_TABLE_ADD: {
        mesh_event_routing_table_change_t *routing_table = (mesh_event_routing_table_change_t *)event_data;
        ESP_LOGW(MESH_TAG, "<MESH_EVENT_ROUTING_TABLE_ADD>add %d, new:%d",
                 routing_table->rt_size_change,
                 routing_table->rt_size_new);
    }
    break;
    case MESH_EVENT_ROUTING_TABLE_REMOVE: {
        mesh_event_routing_table_change_t *routing_table = (mesh_event_routing_table_change_t *)event_data;
        ESP_LOGW(MESH_TAG, "<MESH_EVENT_ROUTING_TABLE_REMOVE>remove %d, new:%d",
                 routing_table->rt_size_change,
                 routing_table->rt_size_new);
    }
    break;
    case MESH_EVENT_NO_PARENT_FOUND: {
        mesh_event_no_parent_found_t *no_parent = (mesh_event_no_parent_found_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_NO_PARENT_FOUND>scan times:%d",
                 no_parent->scan_times);
    }
    /* TODO handler for the failure */
    break;
    case MESH_EVENT_PARENT_CONNECTED: {
    	gpio_set_level(LED_PAPA,1);
        mesh_event_connected_t *connected = (mesh_event_connected_t *)event_data;
        esp_mesh_get_id(&id);
        mesh_layer = connected->self_layer;
        memcpy(&mesh_parent_addr.addr, connected->connected.bssid, 6);
        ESP_LOGI(MESH_TAG,
                 "<MESH_EVENT_PARENT_CONNECTED>layer:%d-->%d, parent:"MACSTR"%s, ID:"MACSTR"",
                 last_layer, mesh_layer, MAC2STR(mesh_parent_addr.addr),
                 esp_mesh_is_root() ? "<ROOT>" :
                 (mesh_layer == 2) ? "<layer2>" : "", MAC2STR(id.addr));
        last_layer = mesh_layer;
        is_mesh_connected = true;
        if (esp_mesh_is_root()) {
            tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_STA);
        }
        //esp_mesh_comm_p2p_start();
    }
    break;
    case MESH_EVENT_PARENT_DISCONNECTED: {
    	gpio_set_level(LED_PAPA,0);
        mesh_event_disconnected_t *disconnected = (mesh_event_disconnected_t *)event_data;
        ESP_LOGI(MESH_TAG,
                 "<MESH_EVENT_PARENT_DISCONNECTED>reason:%d",
                 disconnected->reason);
        is_mesh_connected = false;
        mesh_layer = esp_mesh_get_layer();
    }
    break;
    case MESH_EVENT_LAYER_CHANGE: {
        mesh_event_layer_change_t *layer_change = (mesh_event_layer_change_t *)event_data;
        mesh_layer = layer_change->new_layer;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_LAYER_CHANGE>layer:%d-->%d%s",
                 last_layer, mesh_layer,
                 esp_mesh_is_root() ? "<ROOT>" :
                 (mesh_layer == 2) ? "<layer2>" : "");
        last_layer = mesh_layer;
    }
    break;
    case MESH_EVENT_ROOT_ADDRESS: {
        mesh_event_root_address_t *root_addr = (mesh_event_root_address_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_ROOT_ADDRESS>root address:"MACSTR"",
                 MAC2STR(root_addr->addr));
    }
    break;
    case MESH_EVENT_VOTE_STARTED: {
        mesh_event_vote_started_t *vote_started = (mesh_event_vote_started_t *)event_data;
        ESP_LOGI(MESH_TAG,
                 "<MESH_EVENT_VOTE_STARTED>attempts:%d, reason:%d, rc_addr:"MACSTR"",
                 vote_started->attempts,
                 vote_started->reason,
                 MAC2STR(vote_started->rc_addr.addr));
    }
    break;
    case MESH_EVENT_VOTE_STOPPED: {
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_VOTE_STOPPED>");
        break;
    }
    case MESH_EVENT_ROOT_SWITCH_REQ: {
        mesh_event_root_switch_req_t *switch_req = (mesh_event_root_switch_req_t *)event_data;
        ESP_LOGI(MESH_TAG,
                 "<MESH_EVENT_ROOT_SWITCH_REQ>reason:%d, rc_addr:"MACSTR"",
                 switch_req->reason,
                 MAC2STR( switch_req->rc_addr.addr));
    }
    break;
    case MESH_EVENT_ROOT_SWITCH_ACK: {
        /* new root */
        mesh_layer = esp_mesh_get_layer();
        esp_mesh_get_parent_bssid(&mesh_parent_addr);
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_ROOT_SWITCH_ACK>layer:%d, parent:"MACSTR"", mesh_layer, MAC2STR(mesh_parent_addr.addr));
    }
    break;
    case MESH_EVENT_TODS_STATE: {
        mesh_event_toDS_state_t *toDs_state = (mesh_event_toDS_state_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_TODS_REACHABLE>state:%d", *toDs_state);
    }
    break;
    case MESH_EVENT_ROOT_FIXED: {
        mesh_event_root_fixed_t *root_fixed = (mesh_event_root_fixed_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_ROOT_FIXED>%s",
                 root_fixed->is_fixed ? "fixed" : "not fixed");
    }
    break;
    case MESH_EVENT_ROOT_ASKED_YIELD: {
        mesh_event_root_conflict_t *root_conflict = (mesh_event_root_conflict_t *)event_data;
        ESP_LOGI(MESH_TAG,
                 "<MESH_EVENT_ROOT_ASKED_YIELD>"MACSTR", rssi:%d, capacity:%d",
                 MAC2STR(root_conflict->addr),
                 root_conflict->rssi,
                 root_conflict->capacity);
    }
    break;
    case MESH_EVENT_CHANNEL_SWITCH: {
        mesh_event_channel_switch_t *channel_switch = (mesh_event_channel_switch_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_CHANNEL_SWITCH>new channel:%d", channel_switch->channel);
    }
    break;
    case MESH_EVENT_SCAN_DONE: {
        mesh_event_scan_done_t *scan_done = (mesh_event_scan_done_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_SCAN_DONE>number:%d",
                 scan_done->number);
    }
    break;
    case MESH_EVENT_NETWORK_STATE: {
        mesh_event_network_state_t *network_state = (mesh_event_network_state_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_NETWORK_STATE>is_rootless:%d",
                 network_state->is_rootless);
    }
    break;
    case MESH_EVENT_STOP_RECONNECTION: {
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_STOP_RECONNECTION>");
    }
    break;
    case MESH_EVENT_FIND_NETWORK: {
        mesh_event_find_network_t *find_network = (mesh_event_find_network_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_FIND_NETWORK>new channel:%d, router BSSID:"MACSTR"",
                 find_network->channel, MAC2STR(find_network->router_bssid));
    }
    break;
    case MESH_EVENT_ROUTER_SWITCH: {
        mesh_event_router_switch_t *router_switch = (mesh_event_router_switch_t *)event_data;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_ROUTER_SWITCH>new router:%s, channel:%d, "MACSTR"",
                 router_switch->ssid, router_switch->channel, MAC2STR(router_switch->bssid));
    }
    break;
    default:
        ESP_LOGI(MESH_TAG, "unknown id:%d", event_id);
        break;
    }
}

void ip_event_handler(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    ESP_LOGI(MESH_TAG, "<IP_EVENT_STA_GOT_IP>IP:%s", ip4addr_ntoa(&event->ip_info.ip));
}

void app_main(void)
{
	semaforo = xSemaphoreCreateBinary();

   gpio_pad_select_gpio(PULSADOR_PIN);   //configuro el PIN_PULSADOR como un pin GPIO
   gpio_set_direction(PULSADOR_PIN, GPIO_MODE_DEF_INPUT);    // seleciono el PIN_PULSADOR como pin de entrada
   gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);// instala el servicio ISR con la configuración por defecto.
   gpio_isr_handler_add(PULSADOR_PIN, pulsador_isr_handler, NULL); // añado el manejador para el servicio ISR
   gpio_set_intr_type(PULSADOR_PIN,GPIO_INTR_NEGEDGE);  // habilito interrupción por flanco descendente (1->0)
  // gpio_pullup_en(PULSADOR_PIN);
   //gpio_set_pull_mode(PULSADOR_PIN,GPIO_PULLUP_ONLY );
   gpio_pad_select_gpio(LED_PAPA);
   gpio_set_direction(LED_PAPA,GPIO_MODE_DEF_OUTPUT);

   xTaskCreate(botonCounter,"Conteo",1024*2,NULL,5,NULL);
   xTaskCreate(esp_mesh_p2p_rx_main,"recepcion",2048,NULL,5,NULL);

    nvs_flash_init();
    /*  tcpip initialization */
    tcpip_adapter_init();
    /* for mesh
     * stop DHCP server on softAP interface by default
     * stop DHCP client on station interface by default
     * */
   tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
   tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
    /*  event initialization */
   esp_event_loop_create_default();
    /*  wifi initialization */
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&config);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL);
    esp_wifi_set_storage(WIFI_STORAGE_FLASH);
    esp_wifi_start();
    /*  mesh initialization */
    esp_mesh_init();
    esp_event_handler_register(MESH_EVENT, ESP_EVENT_ANY_ID, &mesh_event_handler, NULL);
    esp_mesh_set_max_layer(CONFIG_MESH_MAX_LAYER);
    esp_mesh_set_vote_percentage(1);
    esp_mesh_set_ap_assoc_expire(10);
    mesh_cfg_t cfg = MESH_INIT_CONFIG_DEFAULT();
    /* mesh ID */
    memcpy((uint8_t *) &cfg.mesh_id, MESH_ID, 6);
    /* router */
    cfg.channel = CONFIG_MESH_CHANNEL;
    cfg.router.ssid_len = strlen(CONFIG_MESH_ROUTER_SSID);
    memcpy((uint8_t *) &cfg.router.ssid, CONFIG_MESH_ROUTER_SSID, cfg.router.ssid_len);
    memcpy((uint8_t *) &cfg.router.password, CONFIG_MESH_ROUTER_PASSWD,
           strlen(CONFIG_MESH_ROUTER_PASSWD));
    /* mesh softAP */
    esp_mesh_set_ap_authmode(CONFIG_MESH_AP_AUTHMODE);
    cfg.mesh_ap.max_connection = CONFIG_MESH_AP_CONNECTIONS;
    memcpy((uint8_t *) &cfg.mesh_ap.password, CONFIG_MESH_AP_PASSWD,
           strlen(CONFIG_MESH_AP_PASSWD));
    esp_mesh_set_config(&cfg);
    /* mesh start */
    esp_mesh_start();

}
