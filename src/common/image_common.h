#ifndef	_IMAGE_COMMON_H_
#define	_IMAGE_COMMON_H_

#include "queue.h"

typedef	struct bbox {
	int					min_x;
	int					min_y;
	int					max_x;
	int					max_y;
	double				distance;
	TAILQ_ENTRY(bbox)	link;
} bbox_t;

typedef	TAILQ_HEAD(bbox_list_t, bbox)	bbox_list_t;



#ifdef __cplusplus
extern "C" {
#endif

	
#ifdef __cplusplus
}
#endif
#endif	/* _IMAGE_COMMON_H_ */

