/*
 * 	SnapFind (Release 0.9)
 *      An interactive image search application
 *
 *      Copyright (c) 2002-2005, Intel Corporation
 *      All Rights Reserved
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */


#include "lib_log.h"

#define FILTER_ASSERT(exp,msg)							\
if(!(exp)) {									\
  log_message(LOGT_FILT, LOGL_ERR, "Assertion %s failed at ", #exp);			\
  log_message(LOGT_FILT, LOGL_ERR, "%s, line %d.", __FILE__, __LINE__);		\
  log_message(LOGT_FILT, LOGL_ERR, "(%s)", msg);					\
  fprintf(stderr, "ERROR %s (%s, line %d)\n", (msg), __FILE__, __LINE__);	\
  pass = 0;									\
  goto done;									\
}


/* read: evaluate <error_exp> if(exp) is true. */

#define ASSERTX(error_exp,exp)								\
if(!(exp)) {										\
  log_message(LOGT_FILT, LOGL_ERR, "Assertion %s failed at ", #exp);			\
  log_message(LOGT_FILT, LOGL_ERR, "%s, line %d.", __FILE__, __LINE__);			\
  fprintf(stderr, "ASSERT %s failed (%s, line %d)\n", #exp, __FILE__, __LINE__);	\
  (error_exp);										\
  goto done;										\
}
