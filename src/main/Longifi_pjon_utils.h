#ifndef LONGIFI_PJON_UTILS_H
#define LONGIFI_PJON_UTILS_H


#if defined(FIXED_STA)
#define PJON_LOCAL_ADDRESS    10  // 255 -> unassigned, 0 -> broadcast
#define PJON_REMOTE_ADDRESS   20

#elif defined(MOBILE_AP)
#define PJON_LOCAL_ADDRESS    20
#define PJON_REMOTE_ADDRESS   10

#else
#error "Please define the model role as either: FIXED_STA or MOBILE_AP."
#endif



#endif