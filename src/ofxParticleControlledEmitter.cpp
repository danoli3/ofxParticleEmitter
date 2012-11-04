
#include "ofxParticleControlledEmitter.h"

// ------------------------------------------------------------------------
// Lifecycle
// ------------------------------------------------------------------------

ofxParticleControlledEmitter::ofxParticleControlledEmitter()
{
	particlesFired = 0;
	particlesLeft = 0;
}



// ------------------------------------------------------------------------
// Particle Management
// ------------------------------------------------------------------------

bool ofxParticleControlledEmitter::addParticle()
{
	// If we have already reached the maximum number of particles then do nothing
	if(particleCount == maxParticles)
		return false;
	
	// Take the next particle out of the particle pool we have created and initialize it
	Particle *particle = &particles[particleCount];
	initParticle( particle );
	
	// Increment the particle count
	particleCount++;
	
	// Return true to show that a particle has been created
	return true;
}

void ofxParticleControlledEmitter::initParticle( Particle* particle )
{
	// Init the position of the particle.  This is based on the source position of the particle emitter
	// plus a configured variance.  The RANDOM_MINUS_1_TO_1 macro allows the number to be both positive
	// and negative
	particle->position.x = sourcePosition.x + sourcePositionVariance.x * RANDOM_MINUS_1_TO_1();
	particle->position.y = sourcePosition.y + sourcePositionVariance.y * RANDOM_MINUS_1_TO_1();
    particle->startPos.x = sourcePosition.x;
    particle->startPos.y = sourcePosition.y;
	
	// Init the direction of the particle.  The newAngle is calculated using the angle passed in and the
	// angle variance.
	float newAngle = (GLfloat)DEGREES_TO_RADIANS(angle + angleVariance * RANDOM_MINUS_1_TO_1());
	
	// Create a new Vector2f using the newAngle
	Vector2f vector = Vector2fMake(cosf(newAngle), sinf(newAngle));
	
	// Calculate the vectorSpeed using the speed and speedVariance which has been passed in
	float vectorSpeed = speed + speedVariance * RANDOM_MINUS_1_TO_1();
	
	// The particles direction vector is calculated by taking the vector calculated above and
	// multiplying that by the speed
	particle->direction = Vector2fMultiply(vector, vectorSpeed);
	
	// Set the default diameter of the particle from the source position
	particle->radius = maxRadius + maxRadiusVariance * RANDOM_MINUS_1_TO_1();
	particle->radiusDelta = (maxRadius / particleLifespan) * (1.0 / MAXIMUM_UPDATE_RATE);
	particle->angle = DEGREES_TO_RADIANS(angle + angleVariance * RANDOM_MINUS_1_TO_1());
	particle->degreesPerSecond = DEGREES_TO_RADIANS(rotatePerSecond + rotatePerSecondVariance * RANDOM_MINUS_1_TO_1());
    
    particle->radialAcceleration = radialAcceleration;
    particle->tangentialAcceleration = tangentialAcceleration;
	
	// Calculate the particles life span using the life span and variance passed in
	particle->timeToLive = MAX(0, particleLifespan + particleLifespanVariance * RANDOM_MINUS_1_TO_1());
	
	// Calculate the particle size using the start and finish particle sizes
	GLfloat particleStartSize = startParticleSize + startParticleSizeVariance * RANDOM_MINUS_1_TO_1();
	GLfloat particleFinishSize = finishParticleSize + finishParticleSizeVariance * RANDOM_MINUS_1_TO_1();
	particle->particleSizeDelta = ((particleFinishSize - particleStartSize) / particle->timeToLive) * (1.0 / MAXIMUM_UPDATE_RATE);
	particle->particleSize = MAX(0, particleStartSize);
	
	// Calculate the color the particle should have when it starts its life.  All the elements
	// of the start color passed in along with the variance are used to calculate the star color
	Color4f start = {0, 0, 0, 0};
	start.red = startColor.red + startColorVariance.red * RANDOM_MINUS_1_TO_1();
	start.green = startColor.green + startColorVariance.green * RANDOM_MINUS_1_TO_1();
	start.blue = startColor.blue + startColorVariance.blue * RANDOM_MINUS_1_TO_1();
	start.alpha = startColor.alpha + startColorVariance.alpha * RANDOM_MINUS_1_TO_1();
	
	// Calculate the color the particle should be when its life is over.  This is done the same
	// way as the start color above
	Color4f end = {0, 0, 0, 0};
	end.red = finishColor.red + finishColorVariance.red * RANDOM_MINUS_1_TO_1();
	end.green = finishColor.green + finishColorVariance.green * RANDOM_MINUS_1_TO_1();
	end.blue = finishColor.blue + finishColorVariance.blue * RANDOM_MINUS_1_TO_1();
	end.alpha = finishColor.alpha + finishColorVariance.alpha * RANDOM_MINUS_1_TO_1();
	
	// Calculate the delta which is to be applied to the particles color during each cycle of its
	// life.  The delta calculation uses the life span of the particle to make sure that the 
	// particles color will transition from the start to end color during its life time.  As the game
	// loop is using a fixed delta value we can calculate the delta color once saving cycles in the 
	// update method
	particle->color = start;
	particle->deltaColor.red = ((end.red - start.red) / particle->timeToLive) * (1.0 / MAXIMUM_UPDATE_RATE);
	particle->deltaColor.green = ((end.green - start.green) / particle->timeToLive)  * (1.0 / MAXIMUM_UPDATE_RATE);
	particle->deltaColor.blue = ((end.blue - start.blue) / particle->timeToLive)  * (1.0 / MAXIMUM_UPDATE_RATE);
	particle->deltaColor.alpha = ((end.alpha - start.alpha) / particle->timeToLive)  * (1.0 / MAXIMUM_UPDATE_RATE);
}

void ofxParticleControlledEmitter::stopParticleEmitter()
{
	active = false;
	elapsedTime = 0;
	emitCounter = 0;
}

void ofxParticleControlledEmitter::startParticleEmitter()
{
	if(!active)
	{
		particlesFired = 0;
		particlesLeft = maxParticles;
		active = true;
		elapsedTime = 0;
		emitCounter = 0;
	}
}

void ofxParticleControlledEmitter::fireOnce()
{
	// If the emitter is active and the emission rate is greater than zero then emit
	// particles
	
	startParticleEmitter();
	
	emissionRate = maxParticles / particleLifespan;
	
	GLfloat aDelta = (ofGetElapsedTimeMillis()-lastUpdateMillis)/1000.0f;
	
	if(active && emissionRate) {
		float rate = 1.0f/emissionRate;
		emitCounter += aDelta;
		while(particleCount < maxParticles && emitCounter > rate) {
			addParticle();
			emitCounter -= rate;
			particlesFired++;
			particlesLeft--;
			
		}
		
	}
}

void  ofxParticleControlledEmitter::fireOnce(int x, int y)
{
	// If the emitter is active and the emission rate is greater than zero then emit
	// particles
	
	
	// Set variables
	sourcePosition.x = x;
	sourcePosition.y = y;
	
	startParticleEmitter();
	
	emissionRate = maxParticles / particleLifespan;
	
	GLfloat aDelta = (ofGetElapsedTimeMillis()-lastUpdateMillis)/1000.0f;
	
	if(active && emissionRate) {
		float rate = 1.0f/emissionRate;
		emitCounter += aDelta;
		while(particleCount < maxParticles && emitCounter > rate) {
			addParticle();
			emitCounter -= rate;
			particlesFired++;
			particlesLeft--;
		}
		
		
	}
	
}


void  ofxParticleControlledEmitter::fireOnce(int x, int y, int vx, int vy)
{
	// If the emitter is active and the emission rate is greater than zero then emit
	// particles
	
	
	// Set variables
	sourcePosition.x = x;
	sourcePosition.y = y;
	sourcePositionVariance.x = vx;
	sourcePositionVariance.y = vy;
	
	
	startParticleEmitter();
	
	emissionRate = maxParticles / particleLifespan;
	
	GLfloat aDelta = (ofGetElapsedTimeMillis()-lastUpdateMillis)/1000.0f;
	
	if(active && emissionRate) {
		float rate = 1.0f/emissionRate;
		emitCounter += aDelta;
		while(particleCount < maxParticles && emitCounter > rate) {
			addParticle();
			emitCounter -= rate;
			particlesFired++;
			particlesLeft--;
		}
		
		
	}
	
}

// ------------------------------------------------------------------------
// Update
// ------------------------------------------------------------------------

void ofxParticleControlledEmitter::update()
{
	if ( !active ) return;

	// Calculate the emission rate
	emissionRate = maxParticles / particleLifespan;

	GLfloat aDelta = (ofGetElapsedTimeMillis()-lastUpdateMillis)/1000.0f;
	
	if(active && emissionRate) {
		if(particlesLeft > 0 || (emitterType == 1 &&duration != -1))
		{
			float rate = 1.0f/emissionRate;
			emitCounter += aDelta;
			while(particlesFired < maxParticles && particleCount < maxParticles && emitCounter > rate)
			{
				addParticle();
				emitCounter -= rate;
				particlesFired++;
				particlesLeft--;
			}
			
			if(emitterType == 1) // if radial
			{
				elapsedTime += aDelta;
				if(duration != -1 && duration < elapsedTime)
				{	stopParticleEmitter();
					particlesLeft = 0;
				}
			}
		}
		
	}
	
	// Reset the particle index before updating the particles in this emitter
	particleIndex = 0;
	
	if(particleCount != 0 || (emitterType == 1 &&duration != -1))
	{
	
	// Loop through all the particles updating their location and color
	while(particleIndex < particleCount) {
		
		// Get the particle for the current particle index
		Particle *currentParticle = &particles[particleIndex];
        
        // FIX 1
        // Reduce the life span of the particle
        currentParticle->timeToLive -= aDelta;
		
		// If the current particle is alive then update it
		if(currentParticle->timeToLive > 0) {
			
			// If maxRadius is greater than 0 then the particles are going to spin otherwise
			// they are effected by speed and gravity
			if (emitterType == kParticleTypeRadial) {
				
                // FIX 2
                // Update the angle of the particle from the sourcePosition and the radius.  This is only
				// done of the particles are rotating
				currentParticle->angle += currentParticle->degreesPerSecond * aDelta;
				currentParticle->radius -= currentParticle->radiusDelta;
                
				Vector2f tmp;
				tmp.x = sourcePosition.x - cosf(currentParticle->angle) * currentParticle->radius;
				tmp.y = sourcePosition.y - sinf(currentParticle->angle) * currentParticle->radius;
				currentParticle->position = tmp;
				
				if (currentParticle->radius < minRadius)
					currentParticle->timeToLive = 0;
			} else {
				Vector2f tmp, radial, tangential;
                
                radial = Vector2fZero;
                Vector2f diff = Vector2fSub(currentParticle->startPos, Vector2fZero);
                
                currentParticle->position = Vector2fSub(currentParticle->position, diff);
                
                if (currentParticle->position.x || currentParticle->position.y)
                    radial = Vector2fNormalize(currentParticle->position);
                
                tangential.x = radial.x;
                tangential.y = radial.y;
                radial = Vector2fMultiply(radial, currentParticle->radialAcceleration);
                
                GLfloat newy = tangential.x;
                tangential.x = -tangential.y;
                tangential.y = newy;
                tangential = Vector2fMultiply(tangential, currentParticle->tangentialAcceleration);
                
				tmp = Vector2fAdd( Vector2fAdd(radial, tangential), gravity);
                tmp = Vector2fMultiply(tmp, aDelta);
				currentParticle->direction = Vector2fAdd(currentParticle->direction, tmp);
				tmp = Vector2fMultiply(currentParticle->direction, aDelta);
				currentParticle->position = Vector2fAdd(currentParticle->position, tmp);
                currentParticle->position = Vector2fAdd(currentParticle->position, diff);
			}
			
			// Update the particles color
			currentParticle->color.red += currentParticle->deltaColor.red;
			currentParticle->color.green += currentParticle->deltaColor.green;
			currentParticle->color.blue += currentParticle->deltaColor.blue;
			currentParticle->color.alpha += currentParticle->deltaColor.alpha;
			
			// Place the position of the current particle into the vertices array
			vertices[particleIndex].x = currentParticle->position.x;
			vertices[particleIndex].y = currentParticle->position.y;
			
			// Place the size of the current particle in the size array
			currentParticle->particleSize += currentParticle->particleSizeDelta;
			vertices[particleIndex].size = MAX(0, currentParticle->particleSize);
			
			// Place the color of the current particle into the color array
			vertices[particleIndex].color = currentParticle->color;
			
			// Update the particle counter
			particleIndex++;
		} else {
			
			// As the particle is not alive anymore replace it with the last active particle 
			// in the array and reduce the count of particles by one.  This causes all active particles
			// to be packed together at the start of the array so that a particle which has run out of
			// life will only drop into this clause once
			if(particleIndex != particleCount - 1)
				particles[particleIndex] = particles[particleCount - 1];
			particleCount--;
		}
	}
		
	}
	else
	{
		//stopParticleEmitter();
	}

	lastUpdateMillis = ofGetElapsedTimeMillis();
}
