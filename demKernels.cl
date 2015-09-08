#ifndef __OPENCL_VERSION__
#define __kernel
#define __global
#define __local
#define __constant
#define false 0
#define true 1
typedef unsigned int uint;
typedef struct float2 {
	float x, y;
} float2;
typedef struct int2 {
	int x, y;
} int2;
#endif

struct kernelParameters{
	float2 lxly;
	int2 cellnxny;
	float timestep;
	float timestepsq;
	float radius;
	float radiussq;
	float mass;
	float springConstant;
	float verletLength;
	float verletLengthSq;
	float verletDistance;
	float verletDistanceSq;
	float2 cellLengthXY;
	int maxparticlePerCell;
	int verletlistLength;
	int maxTimesteps;
	int snapshot;
	int Nparticle;
	float damping;
};
inline float2 wrappedDistance(__global float2 *p1, __global float2 *p2,
		const float2 *LXLY) {
	float2 tmpdist = *p1 - *p2;
	if (tmpdist.x > 0.5f * (*LXLY).x)
		tmpdist.x -= (*LXLY).x;
	else
		if (tmpdist.x < -0.5f * (*LXLY).x)
				tmpdist.x += (*LXLY).x;
	if (tmpdist.y > 0.5f * (*LXLY).y)
		tmpdist.y -= (*LXLY).y;
	else
		if (tmpdist.y < -0.5 * (*LXLY).y)
			tmpdist.y += (*LXLY).y;
	return tmpdist;
}

inline float2 wrappedDistance_const(__global float2 *p1, __global float2 *p2,
		__constant float2 *LXLY) {
	float2 tmpdist = *p1 - *p2;
		if (tmpdist.x > 0.5f * (*LXLY).x)
			tmpdist.x -= (*LXLY).x;
		else
			if (tmpdist.x < -0.5f * (*LXLY).x)
					tmpdist.x += (*LXLY).x;
		if (tmpdist.y > 0.5f * (*LXLY).y)
			tmpdist.y -= (*LXLY).y;
		else
			if (tmpdist.y < -0.5 * (*LXLY).y)
				tmpdist.y += (*LXLY).y;
		return tmpdist;

}


inline float2 wrappedDistance_rg(float2 *p1, __global float2 *p2,
		__constant float2 *LXLY) {
	float2 tmpdist = *p1 - *p2;
		if (tmpdist.x > 0.5f * (*LXLY).x)
			tmpdist.x -= (*LXLY).x;
		else
			if (tmpdist.x < -0.5f * (*LXLY).x)
					tmpdist.x += (*LXLY).x;
		if (tmpdist.y > 0.5f * (*LXLY).y)
			tmpdist.y -= (*LXLY).y;
		else
			if (tmpdist.y < -0.5 * (*LXLY).y)
				tmpdist.y += (*LXLY).y;
		return tmpdist;
}


__kernel void verletStep1(__global float2 *position, __global float2 *velocity,
		__global float2 *acceleration, __global float2 *oldpositions,
		__constant struct kernelParameters* paras,
		__global uint *verletNeedUpdate) {
	int idx = get_global_id(0);
	int ldx = get_local_id(0);
	float2 localPosition=position[idx];
	localPosition+= velocity[idx] * (paras->timestep)
			+ (0.5f * paras->timestepsq) * acceleration[idx];
	if(localPosition.x < 0) localPosition.x+=paras->lxly.x;
	if(localPosition.x > paras->lxly.x) localPosition.x-=paras->lxly.x;
	if(localPosition.y < 0) localPosition.y+=paras->lxly.y;
	if(localPosition.y > paras->lxly.y) localPosition.y-=paras->lxly.y;			
	//localPosition = fmod(localPosition + paras->lxly, paras->lxly); // wrap position
	velocity[idx] += paras->timestep * acceleration[idx];
	if ( !(*verletNeedUpdate)) {
		if(length(wrappedDistance_rg(&localPosition, &oldpositions[idx],&(paras->lxly)))
				>= paras->verletDistance)
			*verletNeedUpdate = true;
	}
	position[idx]=localPosition;
}

__kernel void ereaseCells(__global int *cells, __global int *verletNeedUpdate,
		__constant struct kernelParameters* paras) {
//	if (*verletNeedUpdate) {
		int ix = get_global_id(0);
		int iy = get_global_id(1);
		cells[iy * paras->cellnxny.x * paras->maxparticlePerCell + ix * paras->maxparticlePerCell] = 0;
//	}
}

__kernel void buildVerlet1(__global float2 *positions,
		__global float2 *oldpositions, __global int *cells,
		__global int *verletNeedUpdate, __constant struct kernelParameters* paras) {
//	if (*verletNeedUpdate) {
		int i = get_global_id(0);
		//printf("verlet 1 %i \n",i);
		oldpositions[i] = positions[i];
		int nx = (int) positions[i].x / paras->cellLengthXY.x;
		int ny = (int) positions[i].y / paras->cellLengthXY.y;
		int cellindex = ny * paras->cellnxny.x * paras->maxparticlePerCell
				+ nx * paras->maxparticlePerCell; //Startindex fuer Zelle berechnen
		int npcell = atomic_add(&cells[cellindex], 1); //Partikel in Zelle inkrementieren
		//int npcell= ++cells[cellindex];
		cells[cellindex + npcell + 1] = i; //partikelnummer zu zelle dazuf?gen
		//printf("Partik. %i Zelle %i %i Part in Zell: %i \n",i,nx,ny,cells[cellindex]);
//	}
}

__kernel void buildVerlet2(__global float2 *positions, __global int *cells,
		__global int *verletList, __global int *verletNeedUpdate,
		__constant struct kernelParameters* paras) {
	//if (*verletNeedUpdate) {
		int i = get_global_id(0);
		//int verletindex = i * paras->verletlistLength;;
		int numberOfParticlesInList=0;
		//printf("null gesetzt %i %i \n",i,verletList[verletindex]);
		int nx = (int) positions[i].x / paras->cellLengthXY.x;
		int ny = (int) positions[i].y / paras->cellLengthXY.y;
		for (int h = -1; h <= 1; h++) {
			for (int w = -1; w <= 1; w++) { // Schleife ?ber nachbarzellen
				int wnx = (nx + h + paras->cellnxny.x ) % paras->cellnxny.x;
				int wny = (ny + w + paras->cellnxny.y ) % paras->cellnxny.y;
				int cellindex = wny * paras->cellnxny.x * paras->maxparticlePerCell
						+ wnx * paras->maxparticlePerCell; //Startindex fuer Zelle berechnen
				//printf("Part: %i checkcell: %i %i %f %f \n",i,wnx,wny,positions[i].x,positions[i].y);
				for (int pindex = 1; pindex <= cells[cellindex]; pindex++) {
					//printf("teste %i %i \n",i,cells[cellindex+pindex]);
					if (cells[cellindex + pindex] != i) {
						//printf("VL for part. %i vllength: %i particle addes: %i verletindex: %i \n",
						//			i,verletList[verletindex],cells[cellindex+pindex],verletindex);
						int partVerletIndex = cells[cellindex + pindex];
						if (length(
								wrappedDistance_const(&positions[i],
										&positions[partVerletIndex], &paras->lxly))
								<= paras->verletLength) {
							//int numberOfParticlesInList=++verletList[i];
							++numberOfParticlesInList;
							verletList[i + numberOfParticlesInList*paras->Nparticle] =
									partVerletIndex;
						}
					}
				}
			}
			verletList[i]=numberOfParticlesInList;
		}
		//*verletNeedUpdate = false; // böser Fehler,
		//wenn ein wi erst mit arbeiten anfängt, wenn ein anderes schon fertig ist
		//macht er gar nichts mehr
	//}
}

__kernel void verletStep2(__global float2 *positions,
		__global float2 *velocities, __global float2 *accelerations,
		__global int *verletList,
		__global int *verletNeedsUpdate,
		__constant struct kernelParameters* paras) {
	int i = get_global_id(0);
	float2 localAcceleration = (float2)(0.0f,0.0f);
	float2 thisposition = positions[i];
	int localVerletlistlength=verletList[i];
	for (int p = 1; p <= localVerletlistlength; ++p) {
		int pindex = verletList[p*paras->Nparticle+i];
		float2 particleDistance = wrappedDistance_rg(&thisposition,
				&positions[pindex], &paras->lxly);
		float overlapp = fmax(2 * paras->radius - length(particleDistance), 0);
		if(overlapp >0 ) {
			particleDistance = normalize(particleDistance);
			float2 normaldamping=dot(velocities[i]-velocities[pindex],particleDistance)*paras->damping;		
			localAcceleration += ( (paras->springConstant * overlapp-normaldamping) / paras->mass)
						* particleDistance;
		}
	}
	velocities[i] += (0.5f * paras->timestep) * (localAcceleration-accelerations[i]);
	accelerations[i]=localAcceleration;
	if(*verletNeedsUpdate && i==0) *verletNeedsUpdate=false;
}

//eof
