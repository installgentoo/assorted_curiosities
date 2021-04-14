typedef struct{
  uint nPart;
  float pRad,Spring,Shear,Attract,cellSize;} tSimParams;

kernel void gravity(global float2 *d_Pos,global float2 *d_Vel,constant tSimParams *parms)
{
  const uint idx=get_global_id(0);
  float2 pos=d_Pos[idx],vel=d_Vel[idx];
  vel+=(float2)(0.0f,-0.004f)*0.2f;
  pos+=vel*0.2f;

  if(pos.y<-1.0f+parms->pRad){
      pos.y=-1.0f+parms->pRad;
      vel.y*=0.2;}
  if(pos.x<-1.0f+parms->pRad){
      pos.x=-1.0f+parms->pRad;
      vel.x*=0.2;}
  if(pos.x>3.0f-parms->pRad){
      pos.x=3.0f-parms->pRad;
      vel.x*=0.2;}

  d_Pos[idx]=pos;
  d_Vel[idx]=vel;
}

kernel void collide(global float2 *d_Pos,global float2 *d_Vel,constant tSimParams *parms)
{
  const uint idx=get_global_id(0);
  const float2 pos=d_Pos[idx],vel=d_Vel[idx];
  int x,y,z;
  uint hash,j;
  float dotnorm,dist;
  float2 force=(float2)(0.0f,0.0f),relPos,relVel,norm,dotnormv,tanVel;

  for(j=0;j<parms->nPart;j++)
    {
      if(j==idx) continue;
      relPos=d_Pos[j]-pos;
      dist  =fast_length(relPos);

      if(dist<parms->cellSize)
        {
          float hdist=parms->cellSize-dist;

          relVel  =d_Vel[j]-vel;
          norm    =fast_normalize(relPos);
          dotnormv=relVel*norm;
          dotnorm =dotnormv.x+dotnormv.y;
          tanVel  =relVel-dotnorm*norm;
          force  +=-parms->Spring*hdist*norm+parms->Shear*tanVel+parms->Attract*relPos;
        }
    }

  d_Vel[idx]=vel+force;
}
