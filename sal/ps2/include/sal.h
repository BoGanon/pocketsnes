
#ifndef _SAL_H_
#define _SAL_H_

#include <tamtypes.h>
#include <lists.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SAL_INPUT_UP			0x0010
#define SAL_INPUT_DOWN			0x0040
#define SAL_INPUT_LEFT			0x0080
#define SAL_INPUT_RIGHT			0x0020
#define SAL_INPUT_A				0x2000
#define SAL_INPUT_B				0x4000
#define SAL_INPUT_X				0x1000
#define SAL_INPUT_Y				0x8000
#define SAL_INPUT_L				0x0400
#define SAL_INPUT_R				0x0800
#define SAL_INPUT_START			0x0008
#define SAL_INPUT_SELECT		0x0001

#define SAL_SCREEN_WIDTH			320
#define SAL_SCREEN_HEIGHT			256
#define SAL_SCREEN_X_STRIDE_LEFT	(-1)
#define SAL_SCREEN_X_STRIDE_RIGHT	(1)
#define SAL_SCREEN_Y_STRIDE_UP		(-SAL_SCREEN_WIDTH)
#define SAL_SCREEN_Y_STRIDE_DOWN	(SAL_SCREEN_WIDTH)
#define SAL_SCREEN_ROTATED			0
#define SAL_MAX_PATH				255
#define SAL_DIR_SEP					"/"
#define SAL_DIR_SEP_BAD				"\\"
#define SAL_OK						1
#define SAL_ERROR					0
#define SAL_TRUE					1
#define SAL_FALSE					0

#define SAL_RGB(R,G,B) (u16)((B) << 10 | (G) << 5 | (R) << 0 )
#define SAL_RGB_PAL(r,g,b) SAL_RGB(r,g,b)

struct SAL_DIR
{
	int fd;
	u32 needParent;
	s8 *path[SAL_MAX_PATH];
	u32 drivesRead;
};
#include "sal_common.h"

#ifdef __cplusplus
}
#endif

#endif /* _SAL_H_ */
