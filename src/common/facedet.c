#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "facedet.h"
#include "face_data.h"               // static data for Viola/Jones detector


// Array of 38 pointers that points to a Level in the classifier.
// We've hardcoded '38' to make it clear that changing the number
// of levels in the classifier would require you to rewrite the
// 'data.h' file.
// 
static Level    level_list[NUM_LEVELS];

static Level    scale_level_list[NUM_LEVELS];
static double   scale_level;


void            scale_feature(const Feature * ofp, Feature * sfp,
                              double scale);
void            fast_scale_feature(const Feature * ofp, Feature * sfp,
                                   double scale);
static inline int test_region_with_single_level(IImage * ii,
                                                int l,
                                                int xoffset, int yoffset,
                                                double scale, double m);
int             test_region(IImage * ii,
                            int l1, int l2,
                            int xoffset, int yoffset, double scale, double m);



// Reads from data.h
// 
void
init_classifier()
{
    int             i;
    for (i = 0; i < NUM_LEVELS; i++) {
        level_list[i].num_features = num_features_static[i];
        level_list[i].threshold = theta_static[i];
    }

    level_list[0].features = (Feature *) feature_set_0;
    level_list[1].features = (Feature *) feature_set_1;
    level_list[2].features = (Feature *) feature_set_2;
    level_list[3].features = (Feature *) feature_set_3;
    level_list[4].features = (Feature *) feature_set_4;
    level_list[5].features = (Feature *) feature_set_5;
    level_list[6].features = (Feature *) feature_set_6;
    level_list[7].features = (Feature *) feature_set_7;
    level_list[8].features = (Feature *) feature_set_8;
    level_list[9].features = (Feature *) feature_set_9;
    level_list[10].features = (Feature *) feature_set_10;
    level_list[11].features = (Feature *) feature_set_11;
    level_list[12].features = (Feature *) feature_set_12;
    level_list[13].features = (Feature *) feature_set_13;
    level_list[14].features = (Feature *) feature_set_14;
    level_list[15].features = (Feature *) feature_set_15;
    level_list[16].features = (Feature *) feature_set_16;
    level_list[17].features = (Feature *) feature_set_17;
    level_list[18].features = (Feature *) feature_set_18;
    level_list[19].features = (Feature *) feature_set_19;
    level_list[20].features = (Feature *) feature_set_20;
    level_list[21].features = (Feature *) feature_set_21;
    level_list[22].features = (Feature *) feature_set_22;
    level_list[23].features = (Feature *) feature_set_23;
    level_list[24].features = (Feature *) feature_set_24;
    level_list[25].features = (Feature *) feature_set_25;
    level_list[26].features = (Feature *) feature_set_26;
    level_list[27].features = (Feature *) feature_set_27;
    level_list[28].features = (Feature *) feature_set_28;
    level_list[29].features = (Feature *) feature_set_29;
    level_list[30].features = (Feature *) feature_set_30;
    level_list[31].features = (Feature *) feature_set_31;
    level_list[32].features = (Feature *) feature_set_32;
    level_list[33].features = (Feature *) feature_set_33;
    level_list[34].features = (Feature *) feature_set_34;
    level_list[35].features = (Feature *) feature_set_35;
    level_list[36].features = (Feature *) feature_set_36;
    level_list[37].features = (Feature *) feature_set_37;


    scale_level = 0.0;
    for (i = 0; i < NUM_LEVELS; i++) {
        Feature        *new_feature_list;
        int             len;

        len = level_list[i].num_features * sizeof(Feature);
        new_feature_list = (Feature *) malloc(len);
        assert(new_feature_list != NULL);
        memcpy(new_feature_list, level_list[i].features, len);
        scale_level_list[i].num_features = num_features_static[i];
        scale_level_list[i].threshold = theta_static[i];
        scale_level_list[i].features = new_feature_list;
    }



}

void
scale_feature_table(double scale)
{
    int             i, j;

    for (i = 0; i < NUM_LEVELS; i++) {
        for (j = 0; j < level_list[i].num_features; j++) {
            scale_feature(level_list[i].features + j,
                          (Feature *) scale_level_list[i].features + j,
                          scale);
        }
    }
    scale_level = scale;

}






/*
 * This tests the region with the feature.  We assume the feature
 * has been scaled appropriately.
 */
int
test_region_with_single_feature(IImage * ii,
                                const Feature * fp,
                                int xoffset, int yoffset, double m)
{
    int             val = 0;
    int             i;
    int             nb = fp->num_corners;
    for (i = 0; i < nb; i++) {
        Corner         *c = (Corner *) (fp->corner_list + i);
        val += c->weight * II_PROBE(ii, xoffset + c->x, yoffset + c->y);
        /*
         * val += c->weight * ii->data[yoffset + c->y][xoffset + c->x]; 
         */
    }
    return (m * val >= fp->threshold);
}

// Returns whether level 'l' of the face detector believes
// the region defined by (xoffset, yoffset, scale) to contain
// a face.
// 
static inline int
test_region_with_single_level(IImage * ii,
                              int l,
                              int xoffset, int yoffset,
                              double scale, double m)
{

    int             nf = scale_level_list[l].num_features;
    double          c = 0.0;
    int             i;
    /*
     * make sure the featues are at the correct scale 
     */
    assert(scale_level == scale);
    assert(xoffset >= 0);
    assert(yoffset >= 0);

    for (i = 0; i < nf; i++) {
        const Feature  *f = scale_level_list[l].features + i;
        assert(f);
        c += test_region_with_single_feature(ii, f, xoffset, yoffset, m) ?
            f->alpha_p : f->alpha_n;
    }
    return (c >= scale_level_list[l].threshold);
}


int
test_region(IImage * ii,
            int l1, int l2, int xoffset, int yoffset, double scale, double m)
{
    int             l;
    for (l = l1; l <= l2; l++) {
        if (0 ==
            test_region_with_single_level(ii, l, xoffset, yoffset, scale,
                                          m)) {
            return 0;
        }
    }
    return 1;
}


// ofp - original feature pointer
// sfp - scaled feature pointer
// 
void
scale_feature(const Feature * ofp, Feature * sfp, double scale)
{
    // Weights are not changed by scaling
    /*
     * for (int i=0; i<ofp->num_corners; i++) { 
     */
    /*
     * sfp->corner_list[i].weight = ofp->corner_list[i].weight; 
     */
    /*
     * } 
     */
    /*
     * sfp->num_corners = ofp->num_corners; 
     */
    /*
     * sfp->absv = ofp->absv; 
     */
    *sfp = *ofp;

    // Different types of features are scaled differently (sigh)
    // 
    const Corner   *ul = ofp->corner_list + 0;
#define rint(v) (v)
    assert(ul->x >= -1);
    assert(ul->y >= -1);
    int             tlx = (int) rint(scale * (ul->x + 1)) - 1;
    int             tly = (int) rint(scale * (ul->y + 1)) - 1;
    assert(tlx >= -1);
    assert(tly >= -1);

    switch (ofp->num_corners) {
    case 9:
        {
            int             h =
                (int) rint(scale * (ofp->corner_list[3].y - ul->y));
            int             w =
                (int) rint(scale * (ofp->corner_list[1].x - ul->x));
            sfp->corner_list[0].x = tlx;
            sfp->corner_list[0].y = tly;
            sfp->corner_list[1].x = tlx + w;
            sfp->corner_list[1].y = tly;
            sfp->corner_list[2].x = tlx + 2 * w;
            sfp->corner_list[2].y = tly;
            sfp->corner_list[3].x = tlx;
            sfp->corner_list[3].y = tly + h;
            sfp->corner_list[4].x = tlx + w;
            sfp->corner_list[4].y = tly + h;
            sfp->corner_list[5].x = tlx + 2 * w;
            sfp->corner_list[5].y = tly + h;
            sfp->corner_list[6].x = tlx;
            sfp->corner_list[6].y = tly + 2 * h;
            sfp->corner_list[7].x = tlx + w;
            sfp->corner_list[7].y = tly + 2 * h;
            sfp->corner_list[8].x = tlx + 2 * w;
            sfp->corner_list[8].y = tly + 2 * h;
        }
        break;
    case 8:
        {
            if (-1 == ofp->orientation) {   // vertical feature 8
                int             h = (int) rint(scale *
                                               (ofp->corner_list[2].y -
                                                ofp->corner_list[1].y));
                int             w =
                    (int) rint(scale *
                               (ofp->corner_list[1].x -
                                ofp->corner_list[0].x));
                sfp->corner_list[0].x = tlx;
                sfp->corner_list[0].y = tly;
                sfp->corner_list[1].x = tlx + w;
                sfp->corner_list[1].y = tly;
                sfp->corner_list[2].x = tlx;
                sfp->corner_list[2].y = tly + h;
                sfp->corner_list[3].x = tlx + w;
                sfp->corner_list[3].y = tly + h;
                sfp->corner_list[4].x = tlx;
                sfp->corner_list[4].y = tly + 2 * h;
                sfp->corner_list[5].x = tlx + w;
                sfp->corner_list[5].y = tly + 2 * h;
                sfp->corner_list[6].x = tlx;
                sfp->corner_list[6].y = tly + 3 * h;
                sfp->corner_list[7].x = tlx + w;
                sfp->corner_list[7].y = tly + 3 * h;
            } else {            // horizontal feature 8
                int             h = (int) rint(scale *
                                               (ofp->corner_list[4].y -
                                                ofp->corner_list[3].y));
                int             w =
                    (int) rint(scale *
                               (ofp->corner_list[1].x -
                                ofp->corner_list[0].x));
                sfp->corner_list[0].x = tlx;
                sfp->corner_list[0].y = tly;
                sfp->corner_list[1].x = tlx + w;
                sfp->corner_list[1].y = tly;
                sfp->corner_list[2].x = tlx + 2 * w;
                sfp->corner_list[2].y = tly;
                sfp->corner_list[3].x = tlx + 3 * w;
                sfp->corner_list[3].y = tly;
                sfp->corner_list[4].x = tlx;
                sfp->corner_list[4].y = tly + h;
                sfp->corner_list[5].x = tlx + w;
                sfp->corner_list[5].y = tly + h;
                sfp->corner_list[6].x = tlx + 2 * w;
                sfp->corner_list[6].y = tly + h;
                sfp->corner_list[7].x = tlx + 3 * w;
                sfp->corner_list[7].y = tly + h;
            }
        }
        break;
    case 6:
        {
            if (-1 == ofp->orientation) {   // vertical feature 6
                int             h = (int) rint(scale *
                                               (ofp->corner_list[2].y -
                                                ofp->corner_list[1].y));
                int             w =
                    (int) rint(scale *
                               (ofp->corner_list[1].x -
                                ofp->corner_list[0].x));
                sfp->corner_list[0].x = tlx;
                sfp->corner_list[0].y = tly;
                sfp->corner_list[1].x = tlx + w;
                sfp->corner_list[1].y = tly;
                sfp->corner_list[2].x = tlx;
                sfp->corner_list[2].y = tly + h;
                sfp->corner_list[3].x = tlx + w;
                sfp->corner_list[3].y = tly + h;
                sfp->corner_list[4].x = tlx;
                sfp->corner_list[4].y = tly + 2 * h;
                sfp->corner_list[5].x = tlx + w;
                sfp->corner_list[5].y = tly + 2 * h;
            } else {            // horizontal feature 6
                int             h = (int) rint(scale *
                                               (ofp->corner_list[3].y -
                                                ofp->corner_list[2].y));
                int             w =
                    (int) rint(scale *
                               (ofp->corner_list[1].x -
                                ofp->corner_list[0].x));
                sfp->corner_list[0].x = tlx;
                sfp->corner_list[0].y = tly;
                sfp->corner_list[1].x = tlx + w;
                sfp->corner_list[1].y = tly;
                sfp->corner_list[2].x = tlx + 2 * w;
                sfp->corner_list[2].y = tly;
                sfp->corner_list[3].x = tlx;
                sfp->corner_list[3].y = tly + h;
                sfp->corner_list[4].x = tlx + w;
                sfp->corner_list[4].y = tly + h;
                sfp->corner_list[5].x = tlx + 2 * w;
                sfp->corner_list[5].y = tly + h;
            }
        }
        break;
    default:
        exit(1);                // SERIOUS ERROR!!
        break;
    };
#undef rint
}
