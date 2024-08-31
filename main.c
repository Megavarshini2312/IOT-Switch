#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_http_client.h"
#include "esp_mac.h"
#include "esp_sntp.h"
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

#define GPIO_7 7       // Define the GPIO pin 7 as input
#define GPIO_6 6       // Define the GPIO pin 6 as input
#define GPIO_5 5       // Define the GPIO pin 5 as input
#define GPIO_4 4       // Define the GPIO pin 4 as input

#define OUTPUT_GPIO_9 9    // Define the GPIO pin 9 as output
#define OUTPUT_GPIO_8 8    // Define the GPIO pin 8 as output
#define OUTPUT_GPIO_19 19  // Define the GPIO pin 19 as output
#define OUTPUT_GPIO_18 18  // Define the GPIO pin 18 as output

static const char *TAG = "HTTP_CLIENT_EXAMPLE";
char buffer[1024];
int relay1, relay2, relay3, relay4;

#define WIFI_SSID "Airtel_9944237235"
#define WIFI_PASS "air64389"

#define FIREBASE_BASE_URL "https://esp32c3-816d6-default-rtdb.asia-southeast1.firebasedatabase.app/IOT_Switches.json"
#define AUTH_TOKEN "A"

// Root CA certificate (replace this with the correct root CA for your Firebase URL)
const char *root_ca = \
"-----BEGIN CERTIFICATE-----\n"
"MIIFYjCCBEqgAwIBAgIQd70NbNs2+RrqIQ/E8FjTDTANBgkqhkiG9w0BAQsFADBX\n"
"MQswCQYDVQQGEwJCRTEZMBcGA1UEChMQR2xvYmFsU2lnbiBudi1zYTEQMA4GA1UE\n"
"CxMHUm9vdCBDQTEbMBkGA1UEAxMSR2xvYmFsU2lnbiBSb290IENBMB4XDTIwMDYx\n"
"OTAwMDA0MloXDTI4MDEyODAwMDA0MlowRzELMAkGA1UEBhMCVVMxIjAgBgNVBAoT\n"
"GUdvb2dsZSBUcnVzdCBTZXJ2aWNlcyBMTEMxFDASBgNVBAMTC0dUUyBSb290IFIx\n"
"MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAthECix7joXebO9y/lD63\n"
"ladAPKH9gvl9MgaCcfb2jH/76Nu8ai6Xl6OMS/kr9rH5zoQdsfnFl97vufKj6bwS\n"
"iV6nqlKr+CMny6SxnGPb15l+8Ape62im9MZaRw1NEDPjTrETo8gYbEvs/AmQ351k\n"
"KSUjB6G00j0uYODP0gmHu81I8E3CwnqIiru6z1kZ1q+PsAewnjHxgsHA3y6mbWwZ\n"
"DrXYfiYaRQM9sHmklCitD38m5agI/pboPGiUU+6DOogrFZYJsuB6jC511pzrp1Zk\n"
"j5ZPaK49l8KEj8C8QMALXL32h7M1bKwYUH+E4EzNktMg6TO8UpmvMrUpsyUqtEj5\n"
"cuHKZPfmghCN6J3Cioj6OGaK/GP5Afl4/Xtcd/p2h/rs37EOeZVXtL0m79YB0esW\n"
"CruOC7XFxYpVq9Os6pFLKcwZpDIlTirxZUTQAs6qzkm06p98g7BAe+dDq6dso499\n"
"iYH6TKX/1Y7DzkvgtdizjkXPdsDtQCv9Uw+wp9U7DbGKogPeMa3Md+pvez7W35Ei\n"
"Eua++tgy/BBjFFFy3l3WFpO9KWgz7zpm7AeKJt8T11dleCfeXkkUAKIAf5qoIbap\n"
"sZWwpbkNFhHax2xIPEDgfg1azVY80ZcFuctL7TlLnMQ/0lUTbiSw1nH69MG6zO0b\n"
"9f6BQdgAmD06yK56mDcYBZUCAwEAAaOCATgwggE0MA4GA1UdDwEB/wQEAwIBhjAP\n"
"BgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBTkrysmcRorSCeFL1JmLO/wiRNxPjAf\n"
"BgNVHSMEGDAWgBRge2YaRQ2XyolQL30EzTSo//z9SzBgBggrBgEFBQcBAQRUMFIw\n"
"JQYIKwYBBQUHMAGGGWh0dHA6Ly9vY3NwLnBraS5nb29nL2dzcjEwKQYIKwYBBQUH\n"
"MAKGHWh0dHA6Ly9wa2kuZ29vZy9nc3IxL2dzcjEuY3J0MDIGA1UdHwQrMCkwJ6Al\n"
"oCOGIWh0dHA6Ly9jcmwucGtpLmdvb2cvZ3NyMS9nc3IxLmNybDA7BgNVHSAENDAy\n"
"MAgGBmeBDAECATAIBgZngQwBAgIwDQYLKwYBBAHWeQIFAwIwDQYLKwYBBAHWeQIF\n"
"AwMwDQYJKoZIhvcNAQELBQADggEBADSkHrEoo9C0dhemMXoh6dFSPsjbdBZBiLg9\n"
"NR3t5P+T4Vxfq7vqfM/b5A3Ri1fyJm9bvhdGaJQ3b2t6yMAYN/olUazsaL+yyEn9\n"
"WprKASOshIArAoyZl+tJaox118fessmXn1hIVw41oeQa1v1vg4Fv74zPl6/AhSrw\n"
"9U5pCZEt4Wi4wStz6dTZ/CLANx8LZh1J7QJVj2fhMtfTJr9w4z30Z209fOU0iOMy\n"
"+qduBmpvvYuR7hZL6Dupszfnw0Skfths18dG9ZKb59UhvmaSGZRVbNQpsg3BZlvi\n"
"d0lIKO2d1xozclOzgjXPYovJJIultzkMu34qQb9Sz/yilrbCgj8=\n"
"-----END CERTIFICATE-----\n";

esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                snprintf(buffer, sizeof(buffer), "%.*s", evt->data_len, (char*)evt->data);
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGI(TAG, "HTTP_EVENT_REDIRECT");
            break;
        default:
            ESP_LOGI(TAG, "Unhandled HTTP event: %d", evt->event_id);
            break;
    }
    return ESP_OK;
}

/* Event handler for catching system events */
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    static int s_retry_num = 0;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < 5) {
            ESP_LOGI(TAG, "Disconnected from WiFi, retrying...");
            esp_wifi_connect();
            s_retry_num++;
        } else {
            ESP_LOGI(TAG, "Failed to connect to WiFi after 5 attempts.");
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        s_retry_num = 0; // Reset retry count on successful connection
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        char ip_str[16];
        esp_ip4addr_ntoa(&event->ip_info.ip, ip_str, sizeof(ip_str));
        ESP_LOGI(TAG, "Got IP: %s", ip_str);
    }
}

/* Initialize WiFi */
void wifi_init_sta(void) {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    ESP_LOGI(TAG, "Connecting to WiFi SSID: %s", WIFI_SSID);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();
}

void perform_http_put_request(const char *path, int relay1, int relay2, int relay3, int relay4) {
    // Construct the full URL with the base URL, path, and auth token
    char url[256];
    snprintf(url, sizeof(url), "%s?auth=%s", FIREBASE_BASE_URL, AUTH_TOKEN);

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = _http_event_handler,
        .cert_pem = root_ca,
        .timeout_ms = 5000, // Set timeout if needed
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Prepare the JSON payload
    char put_data[128];
    snprintf(put_data, sizeof(put_data), "{\"Relay1\":%d,\"Relay2\":%d,\"Relay3\":%d,\"Relay4\":%d}", relay1, relay2, relay3, relay4);

    esp_http_client_set_method(client, HTTP_METHOD_PUT);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, put_data, strlen(put_data));

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP PUT Status = %d, content_length = %lld",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP PUT request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

void perform_http_get_request(void) {
    char url[256];
    snprintf(url, sizeof(url), "%s?auth=%s", FIREBASE_BASE_URL, AUTH_TOKEN);

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = _http_event_handler,
        .cert_pem = root_ca,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %lld",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
        ESP_LOGI(TAG, "Response: %s", buffer);
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

void Configure_pin(void)  {

    // Configure the button GPIOs and other input GPIOs
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;  // Disable interrupt
    io_conf.mode = GPIO_MODE_INPUT;         // Set as input mode
    io_conf.pull_down_en = 1;               // Enable pull-down
    io_conf.pull_up_en = 0;                 // Disable pull-up

    // Configure GPIO 4, 5, 6, 7 as input with pull-down
    io_conf.pin_bit_mask = (1ULL << GPIO_4) | (1ULL << GPIO_7) | (1ULL << GPIO_6) | (1ULL << GPIO_5);
    gpio_config(&io_conf);

    // Configure GPIO 9, 8, 19, 18 as output with pull-down
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << OUTPUT_GPIO_9) | (1ULL << OUTPUT_GPIO_8) | (1ULL << OUTPUT_GPIO_19) | (1ULL << OUTPUT_GPIO_18);
    io_conf.pull_down_en = 0;  // Disable pull-down for outputs
    io_conf.pull_up_en = 0;    // Enable pull-up
    gpio_config(&io_conf);
}

// Function to manually set the time
void set_time_manually(void) {
    struct timeval tv;
    struct tm tm;
    
    // Set your rough time here, e.g., "2024-08-05 12:00:00"
    tm.tm_year = 2024 - 1900;
    tm.tm_mon = 7;     // August (0-based, so 7 means August)
    tm.tm_mday = 5;
    tm.tm_hour = 12;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    tm.tm_isdst = -1;  // Not considering daylight saving time

    tv.tv_sec = mktime(&tm);
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);
}

void wait_for_ip(void) {
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_netif_ip_info_t ip_info;

    while (true) {
        esp_netif_get_ip_info(netif, &ip_info);
        if (ip_info.ip.addr != 0) {
            ESP_LOGI(TAG, "Got IP Address: %s", ip4addr_ntoa(&ip_info.ip));
            break;
        }
        ESP_LOGI(TAG, "Waiting for IP Address...");
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}


void init_nvs(void)  {
    esp_log_level_set("*", ESP_LOG_DEBUG);
    esp_log_level_set("HTTP_CLIENT", ESP_LOG_DEBUG);
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize Wi-Fi
    wifi_init_sta();

    // Wait for IP address
    wait_for_ip();

    // Manually set the time before SNTP sync
    set_time_manually();
}


void set_var1(int32_t value)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open NVS handle
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return;
    }

    // Write variable
    err = nvs_set_i32(my_handle, "var1", value);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to write var1");
    }

    // Commit written value.
    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to commit");
    }

    // Close
    nvs_close(my_handle);
}

int32_t get_var1(void)
{
    nvs_handle_t my_handle;
    esp_err_t err;
    int32_t value = 0; // Default value in case of an error

    // Open NVS handle
    err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return value;
    }

    // Read variable
    err = nvs_get_i32(my_handle, "var1", &value);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE("NVS", "Failed to read var1");
    }

    // Close
    nvs_close(my_handle);

    return value;
}

// Similarly define set_var2, get_var2, set_var3, get_var3, set_var4, get_var4 functions

void set_var2(int32_t value)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open NVS handle
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return;
    }

    // Write variable
    err = nvs_set_i32(my_handle, "var2", value);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to write var2");
    }

    // Commit written value.
    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to commit");
    }

    // Close
    nvs_close(my_handle);
}

int32_t get_var2(void)
{
    nvs_handle_t my_handle;
    esp_err_t err;
    int32_t value = 0; // Default value in case of an error

    // Open NVS handle
    err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return value;
    }

    // Read variable
    err = nvs_get_i32(my_handle, "var2", &value);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE("NVS", "Failed to read var2");
    }

    // Close
    nvs_close(my_handle);

    return value;
}

void set_var3(int32_t value)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open NVS handle
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return;
    }

    // Write variable
    err = nvs_set_i32(my_handle, "var3", value);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to write var3");
    }

    // Commit written value.
    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to commit");
    }

    // Close
    nvs_close(my_handle);
}

int32_t get_var3(void)
{
    nvs_handle_t my_handle;
    esp_err_t err;
    int32_t value = 0; // Default value in case of an error

    // Open NVS handle
    err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return value;
    }

    // Read variable
    err = nvs_get_i32(my_handle, "var3", &value);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE("NVS", "Failed to read var3");
    }

    // Close
    nvs_close(my_handle);

    return value;
}

void set_var4(int32_t value)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open NVS handle
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return;
    }

    // Write variable
    err = nvs_set_i32(my_handle, "var4", value);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to write var4");
    }

    // Commit written value.
    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to commit");
    }

    // Close
    nvs_close(my_handle);
}

int32_t get_var4(void)
{
    nvs_handle_t my_handle;
    esp_err_t err;
    int32_t value = 0; // Default value in case of an error

    // Open NVS handle
    err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return value;
    }

    // Read variable
    err = nvs_get_i32(my_handle, "var4", &value);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE("NVS", "Failed to read var4");
    }

    // Close
    nvs_close(my_handle);

    return value;
}

void parse_relay_values(char *buffer) {
    // Initialize relay values
    relay1 = relay2 = relay3 = relay4 = -1;

    // Parse Relay1 value
    char *pos = strstr(buffer, "\"Relay1\":");
    if (pos != NULL) {
        relay1 = atoi(pos + 9);  // Move 9 characters forward to get the number after "Relay1":
    }

    // Parse Relay2 value
    pos = strstr(buffer, "\"Relay2\":");
    if (pos != NULL) {
        relay2 = atoi(pos + 9);  // Move 9 characters forward to get the number after "Relay2":
    }

    // Parse Relay3 value
    pos = strstr(buffer, "\"Relay3\":");
    if (pos != NULL) {
        relay3 = atoi(pos + 9);  // Move 9 characters forward to get the number after "Relay3":
    }

    // Parse Relay4 value
    pos = strstr(buffer, "\"Relay4\":");
    if (pos != NULL) {
        relay4 = atoi(pos + 9);  // Move 9 characters forward to get the number after "Relay4":
    }

    // Output the results
    printf("Relay1: %d\n", relay1);
    printf("Relay2: %d\n", relay2);
    printf("Relay3: %d\n", relay3);
    printf("Relay4: %d\n", relay4);
}


int32_t get_var1(void);
int32_t get_var2(void);
int32_t get_var3(void);
int32_t get_var4(void);
void set_var1(int32_t value);
void set_var2(int32_t value);
void set_var3(int32_t value);
void set_var4(int32_t value);

void app_main(void) {

    // configure the pins
    Configure_pin();

    // Initialize NVS
    init_nvs();

    // for debuging
    printf("Retrieved values: %ld, %ld, %ld, %ld\n", (get_var1()), (get_var2()), (get_var3()), (get_var4())); 

    // Power Off restore code
    gpio_set_level(OUTPUT_GPIO_9, (get_var1()));
    gpio_set_level(OUTPUT_GPIO_8, (get_var2()));
    gpio_set_level(OUTPUT_GPIO_19, (get_var3()));
    gpio_set_level(OUTPUT_GPIO_18, (get_var4()));

    // Perform HTTP PUT request to update all relay states
    perform_http_put_request("IOT_Switches.json", get_var1(), get_var2(), get_var3(), get_var4());


    
    while (1) {

        // Read the button state and other input states
        int gpio7_state = gpio_get_level(GPIO_7);
        int gpio6_state = gpio_get_level(GPIO_6);
        int gpio5_state = gpio_get_level(GPIO_5);
        int gpio4_state = gpio_get_level(GPIO_4);

        // Codition when touch is triggered
        if(gpio7_state==1) {
            
            gpio_set_level(OUTPUT_GPIO_9, !(get_var1()));
            set_var1(!(get_var1()));
            vTaskDelay(pdMS_TO_TICKS(100)); 
            printf("Retrieved values: %ld, %ld, %ld, %ld\n", (get_var1()), (get_var2()), (get_var3()), (get_var4()));

            // Perform HTTP PUT request to update all relay states
            perform_http_put_request("IOT_Switches.json", get_var1(), get_var2(), get_var3(), get_var4());
    
        }
        else if(gpio6_state==1) {
            gpio_set_level(OUTPUT_GPIO_8, !(get_var2()));
            set_var2(!(get_var2()));
            vTaskDelay(pdMS_TO_TICKS(100));  
            printf("Retrieved values: %ld, %ld, %ld, %ld\n", (get_var1()), (get_var2()), (get_var3()), (get_var4()));

            // Perform HTTP PUT request to update all relay states
            perform_http_put_request("IOT_Switches.json", get_var1(), get_var2(), get_var3(), get_var4());

        }
        else if(gpio5_state==1) {
            gpio_set_level(OUTPUT_GPIO_19, !(get_var3()));
            set_var3(!(get_var3()));
            vTaskDelay(pdMS_TO_TICKS(100)); 
            printf("Retrieved values: %ld, %ld, %ld, %ld\n", (get_var1()), (get_var2()), (get_var3()), (get_var4())); 

            // Perform HTTP PUT request to update all relay states
            perform_http_put_request("IOT_Switches.json", get_var1(), get_var2(), get_var3(), get_var4());

        }
        else if(gpio4_state==1) {
            gpio_set_level(OUTPUT_GPIO_18, !(get_var4()));
            set_var4(!(get_var4()));
            vTaskDelay(pdMS_TO_TICKS(100)); 
            printf("Retrieved values: %ld, %ld, %ld, %ld\n", (get_var1()), (get_var2()), (get_var3()), (get_var4())); 

            // Perform HTTP PUT request to update all relay states
            perform_http_put_request("IOT_Switches.json", get_var1(), get_var2(), get_var3(), get_var4());
            
        }
        // Perform HTTP GET request to update all relay states
        perform_http_get_request();
        // Parse the buffer
        parse_relay_values(buffer);

        set_var1(relay1);
        set_var2(relay2);
        set_var3(relay3);
        set_var4(relay4);

        gpio_set_level(OUTPUT_GPIO_9, (get_var1()));
        gpio_set_level(OUTPUT_GPIO_8, (get_var2()));
        gpio_set_level(OUTPUT_GPIO_19, (get_var3()));
        gpio_set_level(OUTPUT_GPIO_18, (get_var4()));

        // Add a delay to avoid spamming the output
        vTaskDelay(pdMS_TO_TICKS(10));  // 50 ms delay

    }
}
