
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
