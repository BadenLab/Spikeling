# Spikeling

Understanding of how neurons encode and compute information is fundamental to our study of the brain, 
but opportunities for hands-on experience with neurophysiological techniques on live neurons are scarce in science education.
Here, we present Spikeling, an open source £25 in silico implementation of a spiking neuron that mimics a wide range of neuronal 
behaviours for classroom education and public neuroscience outreach.

Spikeling is based on an Arduino microcontroller running the computationally efficient Izhikevich model of a spiking neuron.  
The microcontroller is connected to input ports that simulate synaptic excitation or inhibition, 
dials controlling current injection and noise levels, a photodiode that makes Spikeling light-sensitive and an LED and
speaker that allows spikes to be seen and heard. 
Output ports provide access to variables such as membrane potential for recording in experiments or digital signals that 
can be used to excite other connected Spikelings. 
These features allow for the intuitive exploration of the function of neurons and networks
