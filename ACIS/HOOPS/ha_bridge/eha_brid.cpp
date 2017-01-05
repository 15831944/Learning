// Error code definitions for module "spahbridge/src"

#include "base.hxx"

#include <stdio.h>

#include "errorbase.hxx"
#include "errmsg.hxx"

static message_list spahbridge_ha_bridge_msglst[] =
{
  {"HA_BRIDGE_SEGMENT_DOES_NOT_EXIST", "Given HOOPS key points to a dead segment."},
  {"HA_BRIDGE_ELLIPSE_OUT_OF_RANGE", "Ellipse param value out of range."},
  {"HA_BRIDGE_PARAM_OUT_OF_RANGE", "NURBS param value out of range."},
  {"HA_BRIDGE_HOOPS_INTERNAL_ERROR", "HOOPS internal error."},
  {"HA_BRIDGE_ASM_NOT_SUP", "This version of ConvertEntity does not support assemblies."},
  {"HA_BRIDGE_ASM_NOT_OVR", "This version of ConvertEntity must be overridden for assembly support."},
  {"HA_BRIDGE_INVALID_LICENSE", "The HOOPS license is invalid."},
  {NULL, NULL}
};

#ifdef _MSC_VER
__declspec(dllexport) message_module spahbridge_ha_bridge_errmod("spahbridge/src", spahbridge_ha_bridge_msglst);
#else
message_module spahbridge_ha_bridge_errmod("spahbridge/src", spahbridge_ha_bridge_msglst);
#endif


