/*
 * Copyright (c) 1995 Regents of The University of Michigan.
 * All Rights Reserved.  See COPYRIGHT.
 */
#ifndef _PPD_H
#define _PPD_H

struct ppd_font {
    char		*pd_font;
    struct ppd_font	*pd_next;
};

struct ppd_feature {
    char	*pd_name;
    char	*pd_value;
};

struct ppd_feature	*ppd_feature(int port, char *feature,int len);
struct ppd_font		*ppd_font(int port,char *font);

int ppd_init(int port);

extern struct ppd_font *ppd_fonts;

#endif _PPD_H
