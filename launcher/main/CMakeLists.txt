idf_component_register(
    SRCS "main.c" "home.c" "store.c"
    INCLUDE_DIRS "."
    EMBED_FILES "../../apps/common/icons/folder_community.png" "../../apps/common/icons/store.png"
    REQUIRES board bsp lx_loader lx_devlink lx_comp lx_audio lx_input lx_common lx_update lx_net lx_web lvgl
             nvs_flash heap esp_mm esp_timer esp_driver_tsens fatfs vfs
)

# Limit the external component's own allocation, not just the consumer's
# macro. No changes to the downloaded sources or registry package.
idf_component_get_property(ortho_lib youkorr__lv_draw_ortho COMPONENT_LIB)
target_compile_definitions(${ortho_lib} PUBLIC LV_DRAW_ORTHO_TILE_BYTES=32768)
