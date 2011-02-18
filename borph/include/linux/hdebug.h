/****************************************************************
 * $Id: hdebug.h,v 1.1 2006/01/21 20:48:23 skhay Exp $
 *
 * Define macro for debugging
 *
 * Hayden So
 ****************************************************************/
#ifndef HDEBUG_H
#define HDEBUG_H

/****************************************************************
 * helper functions
 ****************************************************************/
#ifndef HDBG_LVL
#  define HDBG_LVL 0
#endif /* defined(HDBG_LVL) */

#ifndef HDBG_NAME
#  define HDBG_NAME
#endif /* HDBG_NAME */

#ifdef PDEBUG
#undef PDEBUG
#endif

#ifdef HDEBUG
#  define PDEBUG(lvl, fmt, args...) if (lvl <= HDBG_LVL) printk(KERN_DEBUG HDBG_NAME ": " fmt, ## args)
#else /* not in debug mode at all */
#  define PDEBUG(lvl, fmt, args...) /* defined as nothing */
#endif /* HDEBUG */

#ifndef PMSG
#define PMSG(fmt, args...) printk(KERN_INFO HDBG_NAME ": " fmt, ## args)
#endif /* PMSG */


#endif /* HDEBUG_H */
