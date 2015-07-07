/*
 * Copyright (c) 1990,1993 Regents of The University of Michigan.
 * All Rights Reserved. See COPYRIGHT.
 */

/*
 * We have an rtmptab circular linked list for each gateway.  Entries
 * are inserted in the order we get them.  The expectation is that
 * we will get a complexity of N for the stable case.  If we have N
 * existing entries, and M new entries, we'll have on the order of
 * N + ( M * N ) complexity (really it will be something more than
 * that, maybe N + ( M * ( N + 1/2 M )).  Note that having a list to
 * search is superior to a hash table if you are expecting bad data:
 * you have the opportunity to range-check the incoming data.
 *
 * We keep several ZIP related flags and counters here.  For ZIP Extended
 * Replies, we must keep a flag indicating that the zone is up or down.
 * This flag is necessary for ZIP Extended Replies which cross packet
 * boundaries: even tho the rtmptab entry has data, it is not yet
 * complete.  For ZIP in general, we keep a flag indicating that we've
 * asked for a ZIP (E)Reply.  If this flag is not set, we won't process
 * ZIP Reply data for given rtmptab entries.  Lastly, we keep a count of
 * the number of times we've asked for ZIP Reply data.  When this value
 * reaches some value (3?), we can optionally stop asking.
 */
#ifndef _RTMP_H
#define _RTMP_H

#define STARTUP_FIRSTNET	0xff00
#define STARTUP_LASTNET		0xfffe

#endif  _RTMP_H
