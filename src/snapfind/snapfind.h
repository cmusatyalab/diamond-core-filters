#ifndef _SNAP_FIND_H_
#define _SNAP_FIND_H_	1

static const int THUMBSIZE_X = 200;
static const int THUMBSIZE_Y = 150;

/* this is a misnomer; the thumbnail keeps everything we want to know
 * about an image */
typedef struct thumbnail_t {
	RGBImage 	*img;	/* the thumbnail image */
	GtkWidget 	*viewport; /* the viewport that contains the image */
	GtkWidget 	*gimage; /* the image widget */
	TAILQ_ENTRY(thumbnail_t) link;
	char 		name[MAX_NAME];	/* name of image */
	int		nboxes;	/* number of histo regions */
	int             nfaces;	/* number of faces */
	image_hooks_t   *hooks;
	int              marked; /* if the user 'marked' this thumbnail */
	GtkWidget       *frame;
} thumbnail_t;

typedef TAILQ_HEAD(thumblist_t, thumbnail_t) thumblist_t;
extern thumblist_t thumbnails;
extern thumbnail_t *cur_thumbnail;

#ifdef __cplusplus
extern "C" {
#endif

void ss_add_dep(snap_search *dep);
void update_search_entry(snap_search *cur_search, int row);

                                                                                 
#ifdef __cplusplus
}
#endif


#endif	/* ! _SNAP_FIND_H_ */
