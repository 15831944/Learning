// Error code definitions for module "spahpsbridge/src"

#include "base.hxx"

#include <stdio.h>

#include "errorbase.hxx"
#include "errmsg.hxx"

static message_list spahpsbridge_hps_bridge_msglst[] =
{
  {"HPS_MSG_SEGMENT_DOES_NOT_EXIST", "HPS_ACIS: Given HPS::SegmentKey is invalid."},
  {"HPS_MSG_ELLIPSE_OUT_OF_RANGE", "HPS_ACIS: Ellipse param value out of range."},
  {"HPS_MSG_PARAM_OUT_OF_RANGE", "HPS_ACIS: NURBS param value out of range."},
  {"HPS_MSG_HOOPS_INTERNAL_ERROR", "HPS_ACIS: Internal error."},
  {"HPS_MSG_ASM_NOT_SUP", "HPS_ACIS: This version of ConvertEntity does not support assemblies."},
  {"HPS_MSG_ASM_NOT_OVR", "HPS_ACIS: This version of ConvertEntity must be overridden for assembly support."},
  {"HPS_MSG_INVALID_LICENSE", "HPS_ACIS: The HPS license is invalid."},
  {"HPS_MSG_ERROR", "HPS_ACIS: Bridge error."},
  {NULL, NULL}
};

#ifdef _MSC_VER
__declspec(dllexport) message_module spahpsbridge_hps_bridge_errmod("spahpsbridge/src", spahpsbridge_hps_bridge_msglst);
#else
message_module spahpsbridge_hps_bridge_errmod("spahpsbridge/src", spahpsbridge_hps_bridge_msglst);
#endif


