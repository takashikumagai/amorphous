#include "IK_Jacobian.hpp"
#include <stdlib.h>
#include <math.h>
#include <assert.h>


namespace amorphous
{


///void Arrow(const dVector3& tail, const dVector3& head);

//extern RestPositionOn;
extern dVector3 target[];

// Optimal damping values have to be determined in an ad hoc manner  (Yuck!)
// const double CIK_Jacobian::DefaultDampingLambda = 0.6;		// Optimal for the "Y" shape (any lower gives jitter)
const double CIK_Jacobian::DefaultDampingLambda = 1.1;			// Optimal for the DLS "double Y" shape (any lower gives jitter)
// const double CIK_Jacobian::DefaultDampingLambda = 0.7;			// Optimal for the DLS "double Y" shape with distance clamping (lower gives jitter)

const double CIK_Jacobian::PseudoInverseThresholdFactor = 0.01;
const double CIK_Jacobian::MaxAngleJtranspose = deg_to_rad( 30.0 );
const double CIK_Jacobian::MaxAnglePseudoinverse = deg_to_rad( 5.0 );
const double CIK_Jacobian::MaxAngleDLS = deg_to_rad( 45.0 );
const double CIK_Jacobian::MaxAngleSDLS = deg_to_rad( 45.0 );
const double CIK_Jacobian::BaseMaxTargetDist = 0.4;


CIK_Jacobian::CIK_Jacobian(CIK_Tree* tree)
{
	if( !tree )
		return;

	CIK_Jacobian::tree = tree;
	nEffector = tree->GetNumEffector();
	nJoint = tree->GetNumJoint();
	nRow = 3 * nEffector;
	nCol = nJoint;

	Jend.SetSize(nRow, nCol);				// The Jocobian matrix
	Jend.SetZero();
	Jtarget.SetSize(nRow, nCol);			// The Jacobian matrix based on target positions
	Jtarget.SetZero();
	SetJendActive();

	U.SetSize(nRow, nRow);				// The U matrix for SVD calculations
	w .SetLength(Min(nRow, nCol));
	V.SetSize(nCol, nCol);				// The V matrix for SVD calculations

	dS.SetLength(nRow);			// (Target positions) - (End effector positions)
	dTheta.SetLength(nCol);		// Changes in joint angles
	dPreTheta.SetLength(nCol);

	// Used by Jacobian transpose method & DLS & SDLS
	dT.SetLength(nRow);			// Linearized change in end effector positions based on dTheta

	// Used by the Selectively Damped Least Squares Method
	//dT.SetLength(nRow);
	dSclamp.SetLength(nEffector);
	errorArray.SetLength(nEffector);
	Jnorms.SetSize(nEffector, nCol);		// Holds the norms of the active J matrix

	CurrentUpdateMode = JACOB_JacobianTranspose;

	Reset();
}


void CIK_Jacobian::Reset() 
{
	// Used by Damped Least Squares Method
	DampingLambda = DefaultDampingLambda;
	DampingLambdaSq = Square(DampingLambda);
	// DampingLambdaSDLS = 1.5*DefaultDampingLambda;
	
	dSclamp.Fill(HUGE_VAL);
}


// Compute the deltaS vector, dS, (the error in end effector positions
// Compute the J and K matrices (the Jacobians)
void CIK_Jacobian::ComputeJacobian() 
{
	// Traverse tree to find all end effectors
	dVector3 temp;
	CIK_Node* n = tree->GetRoot();
	while ( n ) {	
		if ( n->IsEffector() ) {
			int i = n->GetEffectorNum();
			const dVector3& targetPos = target[i];

			// Compute the delta S value (differences from end effectors to target positions.
			temp = targetPos;
			temp -= n->GetS();
			dS.SetTriple(i, temp);

			// Find all ancestors (they will usually all be joints)
			// Set the corresponding entries in the Jacobians J, K.
			CIK_Node* m = tree->GetParent(n);
			while ( m ) {
				int j = m->GetJointNum();
				assert ( 0 <=i && i<nEffector && 0<=j && j<nJoint );
				if ( m->IsFrozen() ) {
					Jend.SetTriple(i, j, dVector3(0,0,0));
					Jtarget.SetTriple(i, j, dVector3(0,0,0));
				}
				else {
					temp = m->GetS();			// joint pos.
					temp -= n->GetS();			// -(end effector pos. - joint pos.)
					temp *= m->GetW();			// cross product with joint rotation axis
					Jend.SetTriple(i, j, temp);	
					temp = m->GetS();			// joint pos.
					temp -= targetPos;		// -(target pos. - joint pos.)
					temp *= m->GetW();			// cross product with joint rotation axis
					Jtarget.SetTriple(i, j, temp);	
				}
				m = tree->GetParent( m );
			}
		}
		n = tree->GetSuccessor( n );
	}
}


// The delta theta values have been computed in dTheta array
// Apply the delta theta values to the joints
// Nothing is done about joint limits for now.
void CIK_Jacobian::UpdateThetas() 
{
	// Traverse the tree to find all joints
	// Update the joint angles
	CIK_Node* n = tree->GetRoot();
	while ( n ) {	
		if ( n->IsJoint() ) {
			int i = n->GetJointNum();
			n->AddToTheta( dTheta[i] );
		}
		n = tree->GetSuccessor( n );
	}
	
	// Update the positions and rotation axes of all joints/effectors
	tree->Compute();
}


void CIK_Jacobian::CalcDeltaThetas() 
{
	switch (CurrentUpdateMode)
	{
		case JACOB_Undefined:
			ZeroDeltaThetas();
			break;
		case JACOB_JacobianTranspose:
			CalcDeltaThetasTranspose();
			break;
		case JACOB_PseudoInverse:
			CalcDeltaThetasPseudoinverse();
			break;
		case JACOB_DLS:
			CalcDeltaThetasDLS();
			break;
		case JACOB_SDLS:
			CalcDeltaThetasSDLS();
			break;
	}
}


void CIK_Jacobian::ZeroDeltaThetas()
{
	dTheta.SetZero();
}

// Find the delta theta values using inverse Jacobian method
// Uses a greedy method to decide scaling factor
void CIK_Jacobian::CalcDeltaThetasTranspose()
{
	const dMatrixMN& J = ActiveJacobian();

	J.MultiplyTranspose( dS, dTheta );

	// Scale back the dTheta values greedily 
	J.Multiply ( dTheta, dT );						// dT = J * dTheta
	double alpha = Dot(dS,dT) / dT.NormSq();
	assert ( alpha>0.0 );
	// Also scale back to be have max angle change less than MaxAngleJtranspose
	double maxChange = dTheta.MaxAbs();
	double beta = MaxAngleJtranspose/maxChange;
	dTheta *= Min(alpha, beta);

}


void CIK_Jacobian::CalcDeltaThetasPseudoinverse() 
{	
	dMatrixMN& J = const_cast<dMatrixMN&>(ActiveJacobian());

	// Compute Singular Value Decomposition 
	//	This an inefficient way to do Pseudoinverse, but it is convenient since we need SVD anyway

	J.ComputeSVD( U, w, V );
	
	// Next line for debugging only
    assert(J.DebugCheckSVD(U, w , V));

	// Calculate response vector dTheta that is the DLS solution.
	//	Delta target values are the dS values
	//  We multiply by Moore-Penrose pseudo-inverse of the J matrix
	double pseudoInverseThreshold = PseudoInverseThresholdFactor*w.MaxAbs();

	long diagLength = w.GetLength();
	double* wPtr = w.GetPtr();
	dTheta.SetZero();
	for ( long i=0; i<diagLength; i++ ) {		
		double dotProdCol = U.DotProductColumn( dS, i );		// Dot product with i-th column of U
		double alpha = *(wPtr++);
		if ( fabs(alpha)>pseudoInverseThreshold ) {
			alpha = 1.0/alpha;
			dMatrixMN::AddArrayScale(V.GetNumRows(), V.GetColumnPtr(i), 1, dTheta.GetPtr(), 1, dotProdCol*alpha );
		}
	}

	// Scale back to not exceed maximum angle changes
	double maxChange = dTheta.MaxAbs();
	if ( maxChange>MaxAnglePseudoinverse ) {
		dTheta *= MaxAnglePseudoinverse/maxChange;
	}

}


void CIK_Jacobian::CalcDeltaThetasDLS() 
{	
	const dMatrixMN& J = ActiveJacobian();

	dMatrixMN::MultiplyTranspose(J, J, U);		// U = J * (J^T)
	U.AddToDiagonal( DampingLambdaSq );
	
	// Use the next four lines instead of the succeeding two lines for the DLS method with clamped error vector e.
	// CalcdTClampedFromdS();
	// dVectorN dTextra(3*nEffector);
	// U.Solve( dT, &dTextra );
	// J.MultiplyTranspose( dTextra, dTheta );
	
	// Use these two lines for the traditional DLS method
	U.Solve( dS, &dT );
	J.MultiplyTranspose( dT, dTheta );

	// Scale back to not exceed maximum angle changes
	double maxChange = dTheta.MaxAbs();
	if ( maxChange>MaxAngleDLS ) {
		dTheta *= MaxAngleDLS/maxChange;
	}
}


void CIK_Jacobian::CalcDeltaThetasDLSwithSVD() 
{	
	const dMatrixMN& J = ActiveJacobian();

	// Compute Singular Value Decomposition 
	//	This an inefficient way to do DLS, but it is convenient since we need SVD anyway

	J.ComputeSVD( U, w, V );
	
	// Next line for debugging only
    assert(J.DebugCheckSVD(U, w , V));

	// Calculate response vector dTheta that is the DLS solution.
	//	Delta target values are the dS values
	//  We multiply by DLS inverse of the J matrix
	long diagLength = w.GetLength();
	double* wPtr = w.GetPtr();
	dTheta.SetZero();
	for ( long i=0; i<diagLength; i++ ) {		
		double dotProdCol = U.DotProductColumn( dS, i );		// Dot product with i-th column of U
		double alpha = *(wPtr++);
		alpha = alpha/(Square(alpha)+DampingLambdaSq);
		dMatrixMN::AddArrayScale(V.GetNumRows(), V.GetColumnPtr(i), 1, dTheta.GetPtr(), 1, dotProdCol*alpha );
	}

	// Scale back to not exceed maximum angle changes
	double maxChange = dTheta.MaxAbs();
	if ( maxChange>MaxAngleDLS ) {
		dTheta *= MaxAngleDLS/maxChange;
	}
}


void CIK_Jacobian::CalcDeltaThetasSDLS() 
{	
	const dMatrixMN& J = ActiveJacobian();

	// Compute Singular Value Decomposition 

	J.ComputeSVD( U, w, V );

	// Next line for debugging only
    assert(J.DebugCheckSVD(U, w , V));

	// Calculate response vector dTheta that is the SDLS solution.
	//	Delta target values are the dS values
	int nRows = J.GetNumRows();
	int numEndEffectors = tree->GetNumEffector();		// Equals the number of rows of J divided by three
	int nCols = J.GetNumColumns();
	dTheta.SetZero();

	// Calculate the norms of the 3-vectors in the Jacobian
	long i;
	const double *jx = J.GetPtr();
	double *jnx = Jnorms.GetPtr();
	for ( i=nCols*numEndEffectors; i>0; i-- ) {
		double accumSq = Square(*(jx++));
		accumSq += Square(*(jx++));
		accumSq += Square(*(jx++));
		*(jnx++) = sqrt(accumSq);
	}

	// Clamp the dS values
	CalcdTClampedFromdS();

	// Loop over each singular vector
	for ( i=0; i<nRows; i++ ) {

		double wiInv = w[i];
		if ( NearZero(wiInv,1.0e-10) ) {
			continue;
		}
		wiInv = 1.0/wiInv;

		double N = 0.0;						// N is the quasi-1-norm of the i-th column of U
		double alpha = 0.0;					// alpha is the dot product of dT and the i-th column of U

		const double *dTx = dT.GetPtr();
		const double *ux = U.GetColumnPtr(i);
		long j;
		for ( j=numEndEffectors; j>0; j-- ) {
			double tmp;
			alpha += (*ux)*(*(dTx++));
			tmp = Square( *(ux++) );
			alpha += (*ux)*(*(dTx++));
			tmp += Square(*(ux++));
			alpha += (*ux)*(*(dTx++));
			tmp += Square(*(ux++));
			N += sqrt(tmp);
		}

		// M is the quasi-1-norm of the response to angles changing according to the i-th column of V
		//		Then is multiplied by the wiInv value.
		double M = 0.0;
		double *vx = V.GetColumnPtr(i);
		jnx = Jnorms.GetPtr();
		for ( j=nCols; j>0; j-- ) {
			double accum=0.0;
			for ( long k=numEndEffectors; k>0; k-- ) {
				accum += *(jnx++);
			}
			M += fabs((*(vx++)))*accum;
		}
		M *= fabs(wiInv);
	
		double gamma = MaxAngleSDLS;
		if ( N<M ) {
			gamma *= N/M;				// Scale back maximum permissable joint angle
		}

		// Calculate the dTheta from pure pseudoinverse considerations
		double scale = alpha*wiInv;			// This times i-th column of V is the psuedoinverse response
		dPreTheta.LoadScaled( V.GetColumnPtr(i), scale );
		// Now rescale the dTheta values.
		double max = dPreTheta.MaxAbs();
		double rescale = (gamma)/(gamma+max);
		dTheta.AddScaled(dPreTheta,rescale);
		/*if ( gamma<max) {
			dTheta.AddScaled( dPreTheta, gamma/max );
		}
		else {
			dTheta += dPreTheta;
		}*/
	}

	// Scale back to not exceed maximum angle changes
	double maxChange = dTheta.MaxAbs();
	if ( maxChange>MaxAngleSDLS ) {
		dTheta *= MaxAngleSDLS/(MaxAngleSDLS+maxChange);
		//dTheta *= MaxAngleSDLS/maxChange;
	}
}


void CIK_Jacobian::CalcdTClampedFromdS() 
{
	long len = dS.GetLength();
	long j = 0;
	for ( long i=0; i<len; i+=3, j++ ) {
		double normSq = Square(dS[i])+Square(dS[i+1])+Square(dS[i+2]);
		if ( normSq>Square(dSclamp[j]) ) {
			double factor = dSclamp[j]/sqrt(normSq);
			dT[i] = dS[i]*factor;
			dT[i+1] = dS[i+1]*factor;
			dT[i+2] = dS[i+2]*factor;
		}
		else {
			dT[i] = dS[i];
			dT[i+1] = dS[i+1];
			dT[i+2] = dS[i+2];
		}
	}
}


double CIK_Jacobian::UpdateErrorArray()
{
	double totalError = 0.0;

	// Traverse tree to find all end effectors
	dVector3 temp;
	CIK_Node* n = tree->GetRoot();
	while ( n ) {	
		if ( n->IsEffector() ) {
			int i = n->GetEffectorNum();
			const dVector3& targetPos = target[i];
			temp = targetPos;
			temp -= n->GetS();
			double err = temp.Norm();
			errorArray[i] = err;
			totalError += err;
		}
		n = tree->GetSuccessor( n );
	}
	return totalError;
}


void CIK_Jacobian::UpdatedSClampValue()
{
	// Traverse tree to find all end effectors
	dVector3 temp;
	CIK_Node* n = tree->GetRoot();
	while ( n ) {	
		if ( n->IsEffector() ) {
			int i = n->GetEffectorNum();
			const dVector3& targetPos = target[i];

			// Compute the delta S value (differences from end effectors to target positions.
			// While we are at it, also update the clamping values in dSclamp;
			temp = targetPos;
			temp -= n->GetS();
			double normSi = sqrt(Square(dS[i])+Square(dS[i+1])+Square(dS[i+2]));
			double changedDist = temp.Norm()-normSi;
			if ( changedDist>0.0 ) {
				dSclamp[i] = BaseMaxTargetDist + changedDist;
			}
			else {
				dSclamp[i] = BaseMaxTargetDist;
			}
		}
		n = tree->GetSuccessor( n );
	}
}


void CIK_Jacobian::DrawEigenVectors() const
{/**
	int i, j;
	dVector3 tail;
	dVector3 head;
	Node *node;

	for (i=0; i<w.GetLength(); i++) {
		if ( NearZero( w[i], 1.0e-10 ) ) {
			continue;
		}
		for (j=0; j<nEffector; j++) {
			node = tree->GetEffector(j);
			tail = node->GetS();
			U.GetTriple( j, i, &head );
			head += tail;
			glDisable(GL_LIGHTING);
			glColor3f(1.0f, 0.2f, 0.0f);
			glLineWidth(2.0);
			glBegin(GL_LINES);
			glVertex3f(tail.x, tail.y, tail.z);
			glVertex3f(head.x, head.y, tail.z);
			glEnd();
			Arrow(tail, head);
			glLineWidth(1.0);
			glEnable(GL_LIGHTING);
		}
	}**/
}


void CIK_Jacobian::CompareErrors( const CIK_Jacobian& j1, const CIK_Jacobian& j2, double* weightedDist1, double* weightedDist2 )
{
	const dVectorN& e1 = j1.errorArray;
	const dVectorN& e2 = j2.errorArray;
	double ret1 = 0.0;
	double ret2 = 0.0;
	int len = e1.GetLength();
	for ( long i=0; i<len; i++ ) {
		double v1 = e1[i];
		double v2 = e2[i];
		if ( v1<v2 ) {
			ret1 += v1/v2;
			ret2 += 1.0;
		}
		else if ( v1 != 0.0 ) {
			ret1 += 1.0;
			ret2 += v2/v1;
		}
		else {
			ret1 += 0.0;
			ret2 += 0.0;
		}
	}
	*weightedDist1 = ret1;
	*weightedDist2 = ret2;
}

void CIK_Jacobian::CountErrors( const CIK_Jacobian& j1, const CIK_Jacobian& j2, int* numBetter1, int* numBetter2, int* numTies )
{
	const dVectorN& e1 = j1.errorArray;
	const dVectorN& e2 = j2.errorArray;
	int b1=0, b2=0, tie=0;
	int len = e1.GetLength();
	for ( long i=0; i<len; i++ ) {
		double v1 = e1[i];
		double v2 = e2[i];
		if ( v1<v2 ) {
			b1++;
		}
		else if ( v2<v1 ) {
			b2++;
		}
		else {
			tie++;
		}
	}
	*numBetter1 = b1;
	*numBetter2 = b2;
	*numTies = tie;
}

/* THIS VERSION IS NOT AS GOOD.  DO NOT USE!
void CIK_Jacobian::CalcDeltaThetasSDLSrev2() 
{	
	const dMatrixMN& J = ActiveJacobian();

	// Compute Singular Value Decomposition 

	J.ComputeSVD( U, w, V );
	
	// Next line for debugging only
    assert(J.DebugCheckSVD(U, w , V));

	// Calculate response vector dTheta that is the SDLS solution.
	//	Delta target values are the dS values
	int nRows = J.GetNumRows();
	int numEndEffectors = tree->GetNumEffector();		// Equals the number of rows of J divided by three
	int nCols = J.GetNumColumns();
	dTheta.SetZero();

	// Calculate the norms of the 3-vectors in the Jacobian
	long i;
	const double *jx = J.GetPtr();
	double *jnx = Jnorms.GetPtr();
	for ( i=nCols*numEndEffectors; i>0; i-- ) {
		double accumSq = Square(*(jx++));
		accumSq += Square(*(jx++));
		accumSq += Square(*(jx++));
		*(jnx++) = sqrt(accumSq);
	}

	// Loop over each singular vector
	for ( i=0; i<nRows; i++ ) {

		double wiInv = w[i];
		if ( NearZero(wiInv,1.0e-10) ) {
			continue;
		}

		double N = 0.0;						// N is the quasi-1-norm of the i-th column of U
		double alpha = 0.0;					// alpha is the dot product of dS and the i-th column of U

		const double *dSx = dS.GetPtr();
		const double *ux = U.GetColumnPtr(i);
		long j;
		for ( j=numEndEffectors; j>0; j-- ) {
			double tmp;
			alpha += (*ux)*(*(dSx++));
			tmp = Square( *(ux++) );
			alpha += (*ux)*(*(dSx++));
			tmp += Square(*(ux++));
			alpha += (*ux)*(*(dSx++));
			tmp += Square(*(ux++));
			N += sqrt(tmp);
		}

		// P is the quasi-1-norm of the response to angles changing according to the i-th column of V
		double P = 0.0;
		double *vx = V.GetColumnPtr(i);
		jnx = Jnorms.GetPtr();
		for ( j=nCols; j>0; j-- ) {
			double accum=0.0;
			for ( long k=numEndEffectors; k>0; k-- ) {
				accum += *(jnx++);
			}
			P += fabs((*(vx++)))*accum;
		}
	
		double lambda = 1.0;
		if ( N<P ) {
			lambda -= N/P;				// Scale back maximum permissable joint angle
		}
		lambda *= lambda;
		lambda *= DampingLambdaSDLS;

		// Calculate the dTheta from pure pseudoinverse considerations
		double scale = alpha*wiInv/(Square(wiInv)+Square(lambda));			// This times i-th column of V is the SDLS response
		dMatrixMN::AddArrayScale(nCols, V.GetColumnPtr(i), 1, dTheta.GetPtr(), 1, scale );
	}

	// Scale back to not exceed maximum angle changes
	double maxChange = dTheta.MaxAbs();
	if ( maxChange>MaxAngleSDLS ) {
		dTheta *= MaxAngleSDLS/maxChange;
	}
} */






} // amorphous
