idf_component_register(
    SRCS
        "user_app.cpp"
        "app_services.cpp"
    PRIV_REQUIRES
        ui_bsp
        app_bsp
        port_bsp
        esp_http_client
        esp_http_server
        esp_netif
        esp_hw_support
        nvs_flash
        json
        mbedtls
    INCLUDE_DIRS "./")
