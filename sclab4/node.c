#include <stdio.h>
#include <string.h>
#include "node.h"

int connectcosts[NUMNODES][NUMNODES];

/* Setup the network costs */
void initcosts() {
	static int initialized = 0;
	if (!initialized) {
		/* initialize by hand since not all compilers allow array initilization */
		connectcosts[0][1] = connectcosts[1][0] = 1; // x0
		connectcosts[1][2] = connectcosts[2][1] = 1; // x1
		connectcosts[2][3] = connectcosts[3][2] = 2; // x2
		connectcosts[0][3] = connectcosts[3][0] = 6; // x3
		connectcosts[1][4] = connectcosts[4][1] = 5; // x4
		connectcosts[0][2] = connectcosts[2][0] = 4; // x5
		connectcosts[3][4] = connectcosts[4][3] = 1; // x6

		/* Not connected nodes */
		connectcosts[0][4] = connectcosts[4][0] = connectcosts[1][3] =
			connectcosts[3][1] = connectcosts[2][4] = connectcosts[4][2] =
			999;

		/* Loopback links */
		connectcosts[0][0] = connectcosts[1][1] = connectcosts[2][2] =
			connectcosts[3][3] = connectcosts[4][4] = 0;
	}
}

/**
 * H synarthsh ayth pairnei san orisma enan deikth se Node. To pedio
 * id ths domhs prepei na einai arxikopoihmeno sto index toy komboy (p.x.
 * 0 gia to node 0, 1 gia to node 1, kok) H synarthsh ayth prepei na
 * arxikopoihsei to routing table toy komboy me bash ton pinaka connectcosts
 * poy orizetai kai arxikopoieitai sto node.c kai katopin na steilei ena
 * katallhlo RtPkt se oloys toys geitonikoys komboys toy node.
 */
void initRT(Node* n) {
	// ...
}

/**
 * H synarthsh ayth pairnei san orisma enan deikth se Node kai enan
 * deikth se RtPkt. Prepei na ananewsei to routing table toy Node me
 * bash ta periexomena toy RtPkt kai, an ypar3oyn allages, na steilei ena
 * katallhlo RtPkt se oloys toys geitonikoys komboys toy node.
 */
void updateRT(Node* n, RtPkt* rcvdpkt) {
	// ...
}

/**
 * H synarthsh ayth pairnei san orisma enan deikth se Node. Prepei na
 * typwsei sto standard output to routing table toy node: apostaseis/costs
 * pros toys alloys komboys kai to epomeno bhma/nexthop gia th dromologhsh
 * paketwn pros aytoys.
 */
void printRT(Node* n) {
	// ...
}

