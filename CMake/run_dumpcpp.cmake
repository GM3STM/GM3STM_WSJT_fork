execute_process (
  COMMAND "${DUMPCPP}" -nometaobject -o "${OUTPUT_BASE}" "${TYPE_LIB}"
  RESULT_VARIABLE dumpcpp_result
  )

# Static metadata generation crashes partway through older OmniRig type
# libraries with Qt 6. Runtime metadata keeps the same wrapper API.
if (NOT EXISTS "${OUTPUT_BASE}.h")
  message (FATAL_ERROR "dumpcpp failed (${dumpcpp_result}) and did not generate ${OUTPUT_BASE}.h")
endif ()
