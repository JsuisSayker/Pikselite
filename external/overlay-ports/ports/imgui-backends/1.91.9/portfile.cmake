vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO ocornut/imgui
    REF v1.91.9
    SHA512 c9393bd9f6b49b036ad6ab3ba4d972876c6f60ce7f5c13a7a56ff11b3559ea3211b0caa03eed10b4f4fbe9c371e14f7f24866bd476652f543f3ed3aa878ea930
)

vcpkg_configure_cmake(
    SOURCE_PATH ${CMAKE_CURRENT_LIST_DIR}
    OPTIONS
        -DIMGUI_SOURCE_PATH=${SOURCE_PATH}
    PREFER_NINJA
)

vcpkg_install_cmake()

# License
file(INSTALL
    ${SOURCE_PATH}/LICENSE.txt
    DESTINATION ${CURRENT_PACKAGES_DIR}/share/imgui-backends
    RENAME copyright
)