/*
 * qipc
 *
 * qall.h
 * Created on: 26-Oct-2014
 *      Author: matrix
 */

#ifndef QALL_H_
#define QALL_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <lib.h>
#include <nonblok_all.h>
#include <breceivelib.h>
#include <queuelib.h>
#include <mq_send.h>
#include <mq_reqnotify.h>
#include <mq_receive.h>

#define MAX_LENGTH 100

int get_process_id(char *name);

#endif /* QALL_H_ */
