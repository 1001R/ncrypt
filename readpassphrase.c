#define _GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <paths.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>
#include <string.h>
#include <ctype.h>


static volatile sig_atomic_t signo;

static void handler(int s)
{
	signo = s;
}

char *
readsecret(const char *prompt, char *buf, size_t bufsiz, int echo, int flags)
{
	ssize_t nr;
	int input, output, save_errno;
	char ch, *p, *end;
	struct termios term, oterm;
	struct sigaction sa, savealrm, saveint, savehup, savequit, saveterm;
	struct sigaction savetstp, savettin, savettou, savepipe;

	if (bufsiz == 0) {
		errno = EINVAL;
		return NULL;
	}

restart:
	signo = 0;
	nr = -1;
	save_errno = 0;
	/*
	 * Read and write to /dev/tty if available.  If not, read from
	 * stdin and write to stderr unless a tty is required.
	 */
	if ((input = output = open(_PATH_TTY, O_RDWR)) == -1) {
		input = STDIN_FILENO;
		output = STDERR_FILENO;
	}

	/*
	 * Catch signals that would otherwise cause the user to end
	 * up with echo turned off in the shell.  Don't worry about
	 * things like SIGXCPU and SIGVTALRM for now.
	 */
	sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;		/* don't restart system calls */
	sa.sa_handler = handler;
	
    sigaction(SIGALRM, &sa, &savealrm);
	sigaction(SIGHUP,  &sa, &savehup);
	sigaction(SIGINT,  &sa, &saveint);
	sigaction(SIGPIPE, &sa, &savepipe);
	sigaction(SIGQUIT, &sa, &savequit);
	sigaction(SIGTERM, &sa, &saveterm);
	sigaction(SIGTSTP, &sa, &savetstp);
	sigaction(SIGTTIN, &sa, &savettin);
	sigaction(SIGTTOU, &sa, &savettou);

	/* Turn off echo if possible. */
	if (input != STDIN_FILENO && tcgetattr(input, &oterm) == 0) {
		memcpy(&term, &oterm, sizeof(term));
        term.c_lflag &= ~(ECHO | ECHONL | ICANON);
		tcsetattr(input, TCSAFLUSH, &term);
	} else {
		memset(&term, 0, sizeof(term));
		term.c_lflag |= ECHO;
		memset(&oterm, 0, sizeof(oterm));
		oterm.c_lflag |= ECHO;
	}

	// int _f = open("text.txt", O_CREAT | O_TRUNC | O_WRONLY, 0644);
	/* No I/O if we are already backgrounded. */
	if (signo != SIGTTOU && signo != SIGTTIN) {
		write(output, prompt, strlen(prompt));
		end = buf + bufsiz - 1;
		p = buf;
		while ((nr = read(input, &ch, 1)) == 1 && ch != '\n' && ch != '\r') {
			if (ch == 127) {
				if (p > buf) {
					write(output, "\b\033[K", 4);
					p--;
				}
			} else if (p < end && isprint(ch)) {
				write(output, "*", 1);
				*p++ = ch;
			}
		}
		*p = '\0';
		save_errno = errno;
		if (!(term.c_lflag & ECHO)) {
			write(output, "\n", 1);
		}
	}

	/* Restore old terminal settings and signals. */
	if (memcmp(&term, &oterm, sizeof(term)) != 0) {
		while (tcsetattr(input, TCSAFLUSH, &oterm) == -1 && errno == EINTR) {
			// nop
		}
	}
	sigaction(SIGALRM, &savealrm, NULL);
	sigaction(SIGHUP,  &savehup,  NULL);
	sigaction(SIGINT,  &saveint,  NULL);
	sigaction(SIGQUIT, &savequit, NULL);
	sigaction(SIGPIPE, &savepipe, NULL);
	sigaction(SIGTERM, &saveterm, NULL);
	sigaction(SIGTSTP, &savetstp, NULL);
	sigaction(SIGTTIN, &savettin, NULL);
	sigaction(SIGTTOU, &savettou, NULL);
	if (input != STDIN_FILENO)
		close(input);

	/*
	 * If we were interrupted by a signal, resend it to ourselves
	 * now that we have restored the signal handlers.
	 */
	if (signo) {
		kill(getpid(), signo);
		switch (signo) {
		case SIGTSTP:
		case SIGTTIN:
		case SIGTTOU:
			goto restart;
		}
	}

	if (save_errno) {
		errno = save_errno;
	}
	return(nr == -1 ? NULL : buf);
    return NULL;
}