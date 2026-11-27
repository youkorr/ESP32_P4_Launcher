idf_component_register(
    SRCS "main.c" "home.c" "store.c"
    INCLUDE_DIRS "."
    EMBED_FILES "../../apps/common/icons/folder_community.png" "../../apps/common/icons/store.png"
    REQUIRES board bsp lx_loader lx_devlink lx_comp lx_audio lx_input lx_common lx_update lx_net lx_web lvgl
             nvs_flash heap esp_mm esp_timer esp_driver_tsens fatfs vfs
)

# Limit the external component's own allocation, not just the consumer's
# macro. No changes to the downloaded sources or registry package.
# Registry dependencies use the namespaced IDF name. An override_path
# component is registered under its directory basename.
idf_build_get_property(lx_components BUILD_COMPONENTS)
if("lv_draw_ortho" IN_LIST lx_components)
    set(lx_ortho_component lv_draw_ortho)
elseif("youkorr__lv_draw_ortho" IN_LIST lx_components)
    set(lx_ortho_component youkorr__lv_draw_ortho)
else()
    message(FATAL_ERROR "Ortho external component not found in BUILD_COMPONENTS")
endif()
idf_component_get_property(ortho_lib ${lx_ortho_component} COMPONENT_LIB)
message(STATUS "Ortho component: ${lx_ortho_component}")
target_compile_definitions(${ortho_lib} PUBLIC LV_DRAW_ORTHO_TILE_BYTES=32768)
