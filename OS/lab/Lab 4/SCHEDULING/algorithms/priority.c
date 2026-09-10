#include "../simulator.h"

int priority_scheduler(Simulation *sim)
{
    int selected = -1;
    int selected_effective_priority = 0;

    for (int i = 0; i < sim->process_count; i++) {

        Process *p = &sim->processes[i];

        if (p->state == READY) {	
            int waiting_time = sim->current_time - p->arrival_time;
            int effective_priority = p->priority - (waiting_time / 3);
            if (effective_priority < 1)
		effective_priority = 1;
            if (selected == -1 ||
                effective_priority <
                selected_effective_priority) {

                selected = i;
		selected_effective_priority = effective_priority;
            }
        }
    }

    return selected;
}
