# Math projectiles with PPGA

### PPGA

PPGA stands for Plane-based Projective Geometric Algebra. This let's us apply a unifying geometric ‘all-in’ paradigm (graphics as well as virtual reality)
without conversions to linear algebra routines, aswell as gain performance not by cycle counts, but through writing lesser code volume.

### Project
In this project I’ve been assigned to use 3D PPGA operations and transformations to create the start of a third person 3D game with movement, collisions and interesting camera. 
As my extra feature I’ve decided to use a projectile, that the player can use and throw at different angles. This sphere can interact with their environment and score a
point by hitting the target.
To highlight certain parts, I’ve extracted 3 formulas that I used in the project to create this result.

### The Sandwich Product for Point Transformation
The Formula: nextPos = (T * currentCenter * ~T).Grade3();

I used this to transform the sphere center (a trivector ) based on the translation Motor.
By extracting the third grade, I can assign it again to the sphere.


### The Wedge Product for Distance Calculation
The Formula: distance = (m_Planes[index] ^ currentPos).e0123();
This formula was used multiple times to calculate the distance between 2 objects. In
this project it was used for the collision’s calculations.


### Translation via Line Duality
The Formula: Motor T = Motor::Translation(bulletSpeed * deltaTime, -!bulletDirection);

I used this to create the path my sphere had to follow.

<img width="1200"  alt="image" src="https://github.com/user-attachments/assets/b5458473-0476-4866-b727-d4736f8b55e9" />



<img width="1200"  alt="image" src="https://github.com/user-attachments/assets/ae8bd3b4-b5ae-4d21-b832-801ad7adbd64" />



<img width="1200"  alt="image" src="https://github.com/user-attachments/assets/27ef0213-b5c9-485b-9bee-40f94c95152c" />



# FlyTracer

FlyTracer is the engine used to make this project, provided by the school.
This is a real-time Vulkan compute shader raytracer built for educational purposes at DAE. Uses Projective Geometric Algebra (PGA) via the [FlyFish](https://github.com/fdlombae/FlyFish) library for 3D transformations.

### [Documentation](https://fdlombae.github.io/FlyTracer/)
