/*
* Copyright (c) Bull S.A.  2007 All Rights Reserved.
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of version 2 of the GNU General Public License as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it would be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Further, this software is distributed without any warranty that it is
* free of the rightful claim of any third person regarding infringement
* or the like.  Any license provided herein, whether implied or
* otherwise, applies only to this software file.  Patent licenses, if
* any, provided herein do not apply to combinations of this program with
* other software, or any other product whatsoever.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
* History:
* Created by: Cyril Lacabanne (Cyril.Lacabanne@bull.net)
*
*/

#include <stdio.h>
#include <tirpc/netconfig.h>
#include <sys/socket.h>
#include <tirpc/rpc/rpc.h>
#include <tirpc/rpc/types.h>
#include <tirpc/rpc/xdr.h>
#include <tirpc/rpc/svc.h>
#include <errno.h>

//Standard define
#define VERSNUM 1
#define PROCSIMPLEPING	1

static void exm_proc();

int main(int argn, char *argc[])
{
	//run_mode can switch into stand alone program or program launch by shell script
	int test_status = 1;	//Default test result set to FAILED
	int progNum = atoi(argc[1]);
	SVCXPRT *transp;
	struct netconfig *nconf;

	//Test initialization
	if ((nconf = getnetconfigent("udp")) == NULL) {
		fprintf(stderr, "Cannot get netconfig entry for UDP\n");
		exit(1);
	}
	//Call routine
	transp = svc_tp_create(exm_proc, progNum, VERSNUM, nconf);

	test_status = (transp != NULL) ? 0 : 1;

	//This last printf gives the result status to the tests suite
	//normally should be 0: test has passed or 1: test has failed
	printf("%d\n", test_status);
	return test_status;
}

static void exm_proc(struct svc_req *rqstp, SVCXPRT * transp)
{

}
