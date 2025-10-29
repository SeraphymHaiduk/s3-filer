
set(VENV_PYTHON_EXECUTABLE "${CMAKE_SOURCE_DIR}/env/bin/python")

add_test(NAME Integration.LocalUtilities COMMAND ${VENV_PYTHON_EXECUTABLE} -m pytest -s -vv ${CMAKE_CURRENT_SOURCE_DIR}/local_utilities/local_utilities_test.py)

set_tests_properties(Integration.LocalUtilities
    PROPERTIES 
        LABELS "INTEGRATION"
        ENVIRONMENT "PROJECT_BIN_PATH=$<TARGET_FILE:S3Filer>"    
)

