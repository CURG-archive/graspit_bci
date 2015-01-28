#TODOS

#Grasp Selection State
  * Generate Image Options
    1. Right now, we must always enter refinement, 
       although that is not necessarily meaningful. 

  * Respond Option Choices
    1. There are two versions of the Grasp Selection State. 
       The initial grasp selection state, and the final state. 
       In the initial state, we want to exit follow one signal,
       in the final state we want to follow the other. I have not decided
       yet how to accomplish this.


#Active Refinement State
  * Generate Image Options
    1. We want to generate an executable grasp view that has a special background color from the top choice
    2. Other than that pick the top N reachable grasps with a getAttribute("testResult") > 0
    3. Cache the sent grasps as in the Grasp Selection State
  
  * Respond Option Choices
	1. Take the grasp selected, execute it with the demo hand, and then 'align' the demo hand.
	2. If the executable grasp is selected, then call sort and double check that the 0th grasp 
           has the same ID as the sent hand. Finally, send the exec signal to move to the
           confirmation state. 
	3. If not executable, set a timer to wait for some amount of time. 
	   Then re-enter "Generate Image Options"
    


#Execution state 
  * Generate Image Options
    1. Decide how many faux messages to send (numDistractors
    2. Decide what image to send for them 

  * Respond option choices - This is going to require a lot more work
    1. Send a stop command to stop currently executing trajectory
	*** That this doesn't already happen is a terrible terrible bug. 
    2.  Connect the commands to the existing button callbacks



#Confirmation state
  * 
