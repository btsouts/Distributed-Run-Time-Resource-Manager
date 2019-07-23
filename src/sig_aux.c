#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "include/sig_aux.h"

/* Disable delivery of SIG_INIT and SIG_ACK. */
void
signals_disable(void)
{
	sigset_t sigset;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIG_TIMER);
	sigaddset(&sigset, SIG_CTIMER);
	sigaddset(&sigset, SIG_EPFD_TIMER);
	sigaddset(&sigset, SIG_ITIMER);
	sigaddset(&sigset, SIG_PFD_TIMER);
	if (sigprocmask(SIG_BLOCK, &sigset, NULL) < 0) {
		perror("signals_disable: sigprocmask");
		exit(1);
	}
}

/* Enable delivery of SIG_INIT and SIG_ACK.  */
void signals_enable(void)
{
	sigset_t sigset;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIG_TIMER);
	sigaddset(&sigset, SIG_CTIMER);
	sigaddset(&sigset, SIG_EPFD_TIMER);
	sigaddset(&sigset, SIG_ITIMER);
	sigaddset(&sigset, SIG_PFD_TIMER);

	if (sigprocmask(SIG_UNBLOCK, &sigset, NULL) < 0) {
		perror("signals_enable: sigprocmask");
		exit(1);
	}
}

void
sig_SEGV_enable(void)
{
	sigset_t sigset;
	sigaddset(&sigset, SIGSEGV);
	if (sigprocmask(SIG_UNBLOCK, &sigset, NULL) < 0) {
		perror("signals_enable: sigprocmask");
		exit(1);
	}
}

/* Install two signal handlers.
 * One for SIGCHLD, one for SIGALRM.
 * Make sure both signals are masked when one of them is running.
 */
void install_signal_handlers(void)
{
	sigset_t sigset;
	struct sigaction sa;

	sa.sa_flags = SA_RESTART | SA_SIGINFO;
	sigemptyset(&sigset);
	sa.sa_mask = sigset;

	sa.sa_sigaction = sig_TIMER_handler;
	if (sigaction(SIG_TIMER, &sa, NULL) < 0) {
		perror("sigaction: SIG_TIMER");
		exit(1);
	}
	
	#ifdef BASIC_PAXOS
	sa.sa_sigaction = sig_CTIMER_handler;
	if (sigaction(SIG_CTIMER, &sa, NULL) < 0) {
		perror("sigaction: SIG_CTIMER");
		exit(1);
	}
	
	sa.sa_sigaction = sig_ITIMER_handler;
	if (sigaction(SIG_ITIMER, &sa, NULL) < 0) {
		perror("sigaction: SIG_ITIMER");
		exit(1);
	}
	#endif
	
	#if defined(EPFD) || defined(tEPFD)
	sa.sa_sigaction = sig_EPFD_TIMER_handler;
	if (sigaction(SIG_EPFD_TIMER, &sa, NULL) < 0) {
		perror("sigaction: SIG_EPFD_TIMER");
		exit(1);
	}	
	#endif
	
	#if defined(PFD) || defined(tPFD)
	sa.sa_sigaction = sig_PFD_TIMER_handler;
	if (sigaction(SIG_PFD_TIMER, &sa, NULL) < 0) {
		perror("sigaction: SIG_PFD_TIMER");
		exit(1);
	}
	#endif

}
