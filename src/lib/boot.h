#ifndef __BOOT_H_
#define __BOOT_H_

/**
 * Process event for the main application that a component has finished booting
 *
 * This process event tells the main-application that the component has
 * completely booted up and is ready for beginning it's work.
 *
 * \note the component is not allowed to proceed working until it receives
 * the BOOT_SYSTEM_COMPLETE message as some components it relies on may not
 * be finished booting yet.
 */
#define BOOT_COMPONENT_COMPLETE 201

/**
 * Process event for a component to tell it the system is booting
 *
 * This process event is broadcasted to all components during booting.
 *
 * \note the passed data pointer of the BOOT_SYSTEM_INPROGRESS event
 * is the main application process. It's broadcasted to all to processes
 * that they know where they have to report to if they are finished booting.
 */
#define BOOT_SYSTEM_INPROGRESS 202

/**
 * Process event for a component to start working normally
 *
 * This process event is broadcasted to all components when they all have
 * told the main application that they have finished booting.
 *
 * \note the component is not allowed to proceed working until it receives
 * this message as some components it relies on may not be finished booting
 * yet.
 */
#define BOOT_SYSTEM_COMPLETE 203

/**
 * INTERNAL FUNCTION
 *
 * \note For some reason the BOOT_COMPONENT_WAIT-macro is throwing compile
 * warnings when calling the original process_post-function when first
 * parameter has to be of type "struct process *", even type casting does
 * not help. So casting is done inside this function...
 */
void __process_post(void *p, process_event_t ev, process_data_t data);

/**
 * Boot handler for component processes
 *
 * Components may rely on other components data and therefore need their
 * data structures initialized when calling them. The BOOT_COMPONENT_WAIT
 * function ensures all components have booted and therefore initialized
 * themselves before they are allowed to proceed.
 *
 * \note initialize all your data structures etc. other codes is relying
 * on before calling this function and only proceed after this function
 * finishes. You MAY NOT call functions of any other module or library
 * before this function has finished as you may operate on uninitialized
 * data structures.
 */
#define BOOT_COMPONENT_WAIT(process) \
	static struct process *boot_response = NULL; \
	while(1) { \
		PROCESS_WAIT_EVENT(); \
		if(ev == BOOT_SYSTEM_INPROGRESS && boot_response == NULL) \
			__process_post(boot_response = data, BOOT_COMPONENT_COMPLETE, &process); \
		if(ev == BOOT_SYSTEM_COMPLETE) \
			break; \
	} \


#endif /* __BOOT_H_ */
