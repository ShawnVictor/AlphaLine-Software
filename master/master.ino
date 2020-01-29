float quatDiff(float q[4], float r[4]) {
  float qDiff[4] = {0};
  float rConj[4] = {r[0], -r[1], -r[2], -r[3]};
  float angle;
  qDiff[0] = (rConj[0]*q[0]-rConj[1]*q[1]-rConj[2]*q[2]-rConj[3]*q[3]);
  qDiff[1] = (rConj[0]*q[1]+rConj[1]*q[0]-rConj[2]*q[3]+rConj[3]*q[2]);
  qDiff[2] = (rConj[0]*q[2]+rConj[1]*q[3]+rConj[2]*q[0]-rConj[3]*q[1]);
  qDiff[3] = (rConj[0]*q[3]-rConj[1]*q[2]+rConj[2]*q[1]+rConj[3]*q[0]);
  
  
  float norm = (qDiff[0]*qDiff[0]+qDiff[1]*qDiff[1]+qDiff[2]*qDiff[2]+qDiff[3]*qDiff[3]);
  norm = sqrt(norm);
  
  qDiff[0] /= norm;
  qDiff[1] /= norm;
  qDiff[2] /= norm;
  qDiff[3] /= norm;
  
  angle = 180*2*acos(qDiff[0])/PI;
  return angle;
}

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
