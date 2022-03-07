idf_component_register(
SRCS
    "main.cpp"
    # Add files separately, no recursives or GLOBs as requested by project members.
    "./app/app.cpp"
INCLUDE_DIRS
    "./app/"
)