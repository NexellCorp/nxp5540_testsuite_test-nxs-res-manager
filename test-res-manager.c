/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 * Author: Sungwoo, Park <swpark@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

#include "nxs_function.h"
#include "nxs_ioctl.h"

#define RESMANAGER_DEVICE	"/dev/nxs_res"

static int test_query_devinfo(int fd, int handle)
{
	int ret;
	struct nxs_query_function query;

	bzero(&query, sizeof(query));

	query.handle = handle;
	query.query = NXS_FUNCTION_QUERY_DEVINFO;

	ret = ioctl(fd, NXS_QUERY_FUNCTION, &query);
	if (ret) {
		fprintf(stderr, "%s: failed to NXS_FUNCTION_QUERY\n", __func__);
		return ret;
	}

	fprintf(stdout, "%s: success: dev name is %s for handle %d\n",
		__func__, query.devinfo.dev_name, handle);

	return 0;
}

static int test_remove(int fd, int handle)
{
	int ret;
	struct nxs_remove_function remove;

	bzero(&remove, sizeof(remove));

	remove.handle = handle;

	ret = ioctl(fd, NXS_REMOVE_FUNCTION, &remove);
	if (ret) {
		fprintf(stderr, "failed to NXS_REMOVE_FUNCTION\n");
		return ret;
	}

	fprintf(stdout, "%s: success: handle %d\n", __func__, handle);

	return 0;
}

static int test_multitap(int fd)
{
	int ret;
	struct nxs_request_function req;
	int sibling_handle;
	int handle_first, handle_second;

	bzero(&req, sizeof(req));

	strcpy(req.name, "vip_clipper-multitap-fifo-dmaw");
	/* strcpy(req.name, "cl-mul-fi-dmaw"); */
	req.count = 4;
	req.array[0] = NXS_FUNCTION_VIP_CLIPPER;
	req.array[1] = NXS_FUNCTION_MULTITAP;
	req.array[2] = NXS_FUNCTION_FIFO;
	req.array[3] = NXS_FUNCTION_DMAW;
	req.sibling_handle = -1;

	ret = ioctl(fd, NXS_REQUEST_FUNCTION, &req);
	if (ret) {
		fprintf(stderr, "%s: failed to NXS_REQUEST_FUNCTION\n",
			__func__);
		return ret;
	}

	fprintf(stdout, "%s: success: handle %d, sibling handle %d\n",
		__func__, req.handle, req.sibling_handle);
	handle_first = req.handle;

	sibling_handle = req.sibling_handle;

	bzero(&req, sizeof(req));

	strcpy(req.name, "vip_clipper-multitap-fifo-dmaw");
	req.count = 4;
	req.array[0] = NXS_FUNCTION_VIP_CLIPPER;
	req.array[1] = NXS_FUNCTION_MULTITAP;
	req.array[2] = NXS_FUNCTION_FIFO;
	req.array[3] = NXS_FUNCTION_DMAW;
	req.sibling_handle = sibling_handle;

	ret = ioctl(fd, NXS_REQUEST_FUNCTION, &req);
	if (ret) {
		fprintf(stderr, "%s: failed to NXS_REQUEST_FUNCTION\n",
			__func__);
		return ret;
	}

	fprintf(stdout, "%s: success: handle second %d\n",
		__func__, req.handle);
	handle_second = req.handle;

	test_query_devinfo(fd, handle_first);
	test_query_devinfo(fd, handle_second);

	test_remove(fd, handle_second);
	test_remove(fd, handle_first);

	return 0;
}

static int test_create_dmar_csc_dmaw(int fd)
{
	int ret;
	struct nxs_request_function req;

	bzero(&req, sizeof(req));

	strcpy(req.name, "dmar-csc-dmaw");
	req.count = 3;
	req.array[0] = NXS_FUNCTION_DMAR;
	req.array[1] = NXS_FUNCTION_CSC;
	req.array[2] = NXS_FUNCTION_DMAW;
	req.sibling_handle = -1;

	ret = ioctl(fd, NXS_REQUEST_FUNCTION, &req);
	if (ret) {
		fprintf(stderr, "%s: failed to NXS_REQUEST_FUNCTION\n",
			__func__);
		return ret;
	}

	fprintf(stdout, "%s: success: handle %d\n", __func__, req.handle);

	return req.handle;
}

int main(int argc, char *argv[])
{
	int fd;
	int handle;
	int i;

	fd = open(RESMANAGER_DEVICE, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "failed to open %s\n", RESMANAGER_DEVICE);
		return -ENODEV;
	}

	/* for (i = 0; i < 100; i++) { */
		handle = test_create_dmar_csc_dmaw(fd);

		if (handle >= 0) {
			test_query_devinfo(fd, handle);
			test_remove(fd, handle);
		}
	/* } */

	test_multitap(fd);
	/* test_multitap(fd); */
	/* test_multitap(fd); */

	close(fd);

	return 0;
}
