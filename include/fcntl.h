/*
 * fnctl.h
 *
 *  Created on: 2018Äê6ÔÂ18ÈÕ
 *      Author: crane
 */

#ifndef INCLUDE_FCNTL_H_
#define INCLUDE_FCNTL_H_

extern int open(const char *path, unsigned int flag, unsigned int mode);
extern int read(int fd, char *buff, unsigned int count);

#endif /* INCLUDE_FCNTL_H_ */
