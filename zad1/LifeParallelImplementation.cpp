/*
 * LifeParallelImplementation.cpp
 *
 */

#include "LifeParallelImplementation.h"
#include <stdlib.h>
#include <mpi.h>
#include <algorithm>

using namespace std;

LifeParallelImplementation::LifeParallelImplementation() {}

void LifeParallelImplementation::giveWorkingRange(int rank, int procs, int &start, int &end) {
    int processRangeSize = (size / procs);

    start = rank * processRangeSize;
	end = start + processRangeSize - 1;

    if (rank == procs - 1) {
       end = size_1;
    }
}

void LifeParallelImplementation::sequentionalStep() {
	int currentState, currentPollution;
	for (int row = 1; row < size_1; row++)
		for (int col = 1; col < size_1; col++)
		{
			currentState = cells[row][col];
			currentPollution = pollution[row][col];
			cellsNext[row][col] = rules->cellNextState(currentState, liveNeighbours(row, col),
													   currentPollution);
			pollutionNext[row][col] =
				rules->nextPollution(currentState, currentPollution, pollution[row + 1][col] + pollution[row - 1][col] + pollution[row][col - 1] + pollution[row][col + 1],
									 pollution[row - 1][col - 1] + pollution[row - 1][col + 1] + pollution[row + 1][col - 1] + pollution[row + 1][col + 1]);
		}
}

void LifeParallelImplementation::realStep() {
	int rank, procs;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &procs);
	MPI_Status *status = new MPI_Status();

	int start, end;
	giveWorkingRange(rank, procs, start, end);

	if (rank == 0 ) {
		start = 1;
	}

	if (rank == procs - 1 ) {
		end = size_1 - 1;
	}

	int currentState, currentPollution;
	for (int row = start; row <= end; row++) {
		for (int col = 1; col < size_1; col++) {
			currentState = cells[row][col];
			currentPollution = pollution[row][col];
			cellsNext[row][col] = rules->cellNextState(currentState, liveNeighbours(row, col),
													   currentPollution);
			pollutionNext[row][col] =
				rules->nextPollution(currentState, currentPollution, pollution[row + 1][col] + pollution[row - 1][col] + pollution[row][col - 1] + pollution[row][col + 1],
									 pollution[row - 1][col - 1] + pollution[row - 1][col + 1] + pollution[row + 1][col - 1] + pollution[row + 1][col + 1]);
		}
	}
	
	if (rank == 0) {
		MPI_Send( cellsNext[end], size, MPI_INT, rank + 1, 0, MPI_COMM_WORLD );
		MPI_Send( pollutionNext[end], size, MPI_INT, rank + 1, 1, MPI_COMM_WORLD );
		MPI_Recv( cellsNext[end + 1], size, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, status );
		MPI_Recv( pollutionNext[end + 1], size, MPI_INT, rank + 1, 1, MPI_COMM_WORLD, status );
	} else if (rank == procs - 1) {
		MPI_Recv( cellsNext[start - 1], size, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, status );
		MPI_Recv( pollutionNext[start - 1], size, MPI_INT, rank - 1, 1, MPI_COMM_WORLD, status );
		MPI_Send( cellsNext[start], size, MPI_INT, rank - 1, 0, MPI_COMM_WORLD );
		MPI_Send( pollutionNext[start], size, MPI_INT, rank - 1, 1, MPI_COMM_WORLD );
	} else {
		MPI_Recv( cellsNext[start - 1], size, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, status );
		MPI_Recv( pollutionNext[start - 1], size, MPI_INT, rank - 1, 1, MPI_COMM_WORLD, status );
		MPI_Send( cellsNext[end], size, MPI_INT, rank + 1, 0, MPI_COMM_WORLD );
		MPI_Send( pollutionNext[end], size, MPI_INT, rank + 1, 1, MPI_COMM_WORLD );
		MPI_Recv( cellsNext[end + 1], size, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, status );
		MPI_Recv( pollutionNext[end + 1], size, MPI_INT, rank + 1, 1, MPI_COMM_WORLD, status );
		MPI_Send( cellsNext[start], size, MPI_INT, rank - 1, 0, MPI_COMM_WORLD );
		MPI_Send( pollutionNext[start], size, MPI_INT, rank - 1, 1, MPI_COMM_WORLD );
	}
}

void LifeParallelImplementation::oneStep() {
	int procs;
	MPI_Comm_size(MPI_COMM_WORLD, &procs);

	if (procs == 1) {
		sequentionalStep();
	} else {
		realStep();
	}

	swapTables();
}

void LifeParallelImplementation::beforeFirstStep() {
	int rank, procs;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &procs);
	MPI_Status *status = new MPI_Status();

	if ( !rank ) {
		for ( int proc = 1; proc < procs; proc++ ) {
			int start, end;
			giveWorkingRange(proc, procs, start, end);
			start = max(start - 1, 0);
			end = min(end + 1, size - 1);
			
			for ( int col = start; col <= end; col++ ) {
				MPI_Send( cells[col], size, MPI_INT, proc, 0, MPI_COMM_WORLD );
    		}

			for ( int col = start; col <= end; col++ ) {
				MPI_Send( pollution[col], size, MPI_INT, proc, 0, MPI_COMM_WORLD );
    		}
    	}
    } else {
		int start, end;
		giveWorkingRange(rank, procs, start, end);
		start = max(start - 1, 0);
		end = min(end + 1, size - 1);

		for ( int col = start; col <= end; col++ ) {
			MPI_Recv( cells[col], size, MPI_INT, 0, 0, MPI_COMM_WORLD, status );
		}

		for ( int col = start; col <= end; col++ ) {
			MPI_Recv( pollution[col], size, MPI_INT, 0, 0, MPI_COMM_WORLD, status );
		}
	}
}

void LifeParallelImplementation::afterLastStep() {
	int rank, procs;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &procs);
	MPI_Status *status = new MPI_Status();

	if ( !rank ) {
		for ( int proc = 1; proc < procs; proc++ ) {
			int start, end;
			giveWorkingRange(proc, procs, start, end);

			for ( int col = start; col <= end; col++ ) {
				MPI_Recv( cells[col], size, MPI_INT, proc, 0, MPI_COMM_WORLD, status );
    		}

			for ( int col = start; col <= end; col++ ) {
				MPI_Recv( pollution[col], size, MPI_INT, proc, 0, MPI_COMM_WORLD, status );
    		}
    	}
    } else {
		int start, end;
		giveWorkingRange(rank, procs, start, end);
		
		for ( int col = start; col <= end; col++ ) {
    	    MPI_Send( cells[col], size, MPI_INT, 0, 0, MPI_COMM_WORLD );
		}

		for ( int col = start; col <= end; col++ ) {
			MPI_Send( pollution[col], size, MPI_INT, 0, 0, MPI_COMM_WORLD );
		}
	}
}

int LifeParallelImplementation::numberOfLivingCells() {
	return sumTable( cells );
}

double LifeParallelImplementation::averagePollution() {
	return (double)sumTable( pollution ) / size_1_squared / rules->getMaxPollution();
}