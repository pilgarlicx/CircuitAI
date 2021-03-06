/*
 * Copyright (c) 2000-2003 Lee Thomason (www.grinninglizard.com)
 * Grinning Lizard Utilities.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must
 * not claim that you wrote the original software. If you use this
 * software in a product, an acknowledgment in the product documentation
 * would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 */

/*
 * Updated and changed by Tournesol (on the TA Spring client).
 * May (currenty) only be used to compile KAI (AI for TA Spring).
 * Take care when you use it!
 *
 * This notice may not be removed or altered from any source
 *
 * As of 12 march, 2007, the following applies instead of the above notice:
 * (Tournesol gave permission to change it per e-mail)
 *
 * "All parts of the code in this file that are made by 'Tournesol' are
 * released under GPL licence."
 *
 * --Tobi Vollebregt
 */

/*
 * Changed by rlcevg. Aug 25, 2015
 */


/*
 * @mainpage MicroPather
 *
 * MicroPather is a path finder and A* solver (astar or a-star) written in platform independent
 * C++ that can be easily integrated into existing code. MicroPather focuses on being a path
 * finding engine for video games but is a generic A* solver. MicroPather is open source, with
 * a license suitable for open source or commercial use.
 *
 * An overview of using MicroPather is in the <A HREF="../readme.htm">readme</A> or
 * on the Grinning Lizard website: http://grinninglizard.com/micropather/
 */

#ifndef GRINNINGLIZARD_MICROPATHER_INCLUDED
#define GRINNINGLIZARD_MICROPATHER_INCLUDED

#include <vector>
#include <cfloat>

#ifdef _DEBUG
	#ifndef DEBUG
		#define DEBUG
	#endif
#endif

#define FLT_BIG (FLT_MAX / 2.0)

/*
 * USE_LIST and USE_BINARY_HASH change the some of details the pather algorithms. They
 * are set optimally in my experiments, but different applications may benefit
 * from a different combination of these #defines.
 */

// #define USE_LIST
// #define USE_BINARY_HASH

namespace NSMicroPather {
	/*
	 * A pure abstract class used to define a set of callbacks.
	 * The client application inherits from
	 * this class, and the methods will be called when MicroPather::Solve() is invoked.
	 *
	 * The notion of a "state" is very important. It must have the following properties:
	 * - Unique
	 * - Unchanging (unless MicroPather::Reset() is called)
	 *
	 * If the client application represents states as objects, then the state is usually
	 * just the object cast to a void*. If the client application sees states as numerical
	 * values, (x, y) for example, then state is an encoding of these values. MicroPather
	 * never interprets or modifies the value of state.
	 */

	class Graph {
		public:
			// KLOOTNOTE: declaring a pure virtual destructor here results in
			// Spring load-time error when AI compiled with gcc and link-time
			// error when AI compiled with mingw32

		//	virtual ~Graph() = 0;
			virtual ~Graph() {}

			/*
			 * This function is only used in DEBUG mode - it dumps output to stdout. Since void*
			 * aren't really human readable, normally you print out some concise info (like "(1,2)")
			 * without an ending newline.
			 * @note If you are using other grinning lizard utilities, you should use GLOUTPUT for output.
			 */
		//	virtual void PrintStateInfo(void* state) = 0;
		//	virtual void PrintData(string s) = 0;
		};



	class PathNode {
		// trashy trick to get rid of compiler warning because this class has a private constructor and destructor
		// (it can never be "new" or created on the stack, only by special allocators)
		friend class none;

		public:
			void Init(unsigned _frame, float _costFromStart, PathNode* _parent) {
				costFromStart = _costFromStart;
				totalCost = _costFromStart;
				checkIdx = 0;
				parent = _parent;
				frame = _frame;

				#ifdef USE_BINARY_HASH
				right = 0;
				#endif
				#ifdef USE_LIST
				next = 0;
				prev = 0;
				#endif
				inOpen = 0;
				inClosed = 0;
				isTarget = 0;
				isEndNode = 0;
			}

			inline void Reuse(unsigned _frame) {
				costFromStart = (FLT_BIG / 2.0f);
				checkIdx = 0;
				parent = 0;
				frame = _frame;

				inOpen = 0;
				inClosed = 0;
			}

			int myIndex;
			float costFromStart;	// exact
			float totalCost;		// could be a function, but save some math.
			unsigned checkIdx;		// index of current predicate
			PathNode* parent;		// the parent is used to reconstruct the path

			// Binary tree, where the 'state' is what is being compared.
			// Also used as a "next" pointer for memory layout.

			#ifdef USE_BINARY_HASH
			PathNode* right;
			#endif
			#ifdef USE_LIST
			PathNode* next, *prev;
			#endif

			unsigned inOpen:1;
			unsigned inClosed:1;
			unsigned isTarget:1;
			unsigned isEndNode:1;	// this must be cleared by the call that sets it
			unsigned frame:16;		// this might be set to more that 16

			#ifdef USE_LIST
			void Unlink() {
				next -> prev = prev;
				prev -> next = next;
				next = prev = 0;
			}
			void AddBefore(PathNode* addThis) {
				addThis -> next = this;
				addThis -> prev = prev;
				prev -> next = addThis;
				prev = addThis;
			}
			#ifdef DEBUG
			void CheckList() {
				assert(totalCost == FLT_BIG);

				for (PathNode* it = next; it != this; it = it -> next) {
					assert(it -> prev == this || it -> totalCost >= it -> prev -> totalCost);
					assert(it -> totalCost <= it -> next -> totalCost);
				}
			}
			#endif
			#endif

		private:
			PathNode();
			~PathNode();
		};


	// create a MicroPather object to solve for a best path
	class CMicroPather {
		friend class NSMicroPather::PathNode;

		public:
			enum {
				SOLVED,
				NO_SOLUTION,
				START_END_SAME,
			};

			/*
			 * Construct the pather, passing a pointer to the object that implements the Graph callbacks.
			 *
			 * @param graph		The "map" that implements the Graph callbacks.
			 * @param sizeX		Width of the map.
			 * @param sizeY		Height of the map.
			 */
			CMicroPather(Graph* graph, int sizeX, int sizeY);
			~CMicroPather();

			PathNode* GetNode(int indexNode) const { return &pathNodeMem[indexNode]; }

			/*
			 * Solve for the path from start to end.
			 *
			 * @param startState	Input, the starting state for the path.
			 * @param endState		Input, the ending state for the path.
			 * @param path			Output, a vector of states that define the path. Empty if not found.
			 * @param totalCost	Output, the cost of the path, if found.
			 * @return				Success or failure, expressed as SOLVED, NO_SOLUTION, or START_END_SAME.
			 */
			int Solve(void* startState, void* endState, std::vector<void*>* path, float* totalCost);

			// Should not be called unless there is danger for frame overflow (16bit atm)
			void Reset();

			/**
			  * Return the "checksum" of the last path returned by Solve(). Useful for debugging,
			  * and a quick way to see if 2 paths are the same.
			  */
			unsigned Checksum() const { return checksum; }

			// Tournesol's stuff
			unsigned int* lockUpCount;
			bool* canMoveArray;
			float* costArray;
			int mapSizeX;
			int mapSizeY;
			int offsets[8];
			int xEndNode, yEndNode;
			bool isRunning;
			void SetMapData(bool* canMoveArray, float* costArray);
			int FindBestPathToAnyGivenPoint(void* startNode, std::vector<void*>& endNodes, std::vector<void*>& targets,
											std::vector<void*>* path, float* cost);
			int FindBestPathToAnyGivenPointSafe(void* startNode, std::vector<void*>& endNodes, std::vector<void*>& targets,
											std::vector<void*>* path, float* cost);
			int FindBestPathToPointOnRadius(void* startNode, void* endNode, std::vector<void*>* path, float* cost, int radius);
			int FindBestPathToPointOnRadius(void* startNode, void* endNode, std::vector<void*>* path, float* cost, int radius, float threat);
			int FindBestCostToPointOnRadius(void* startNode, void* endNode, float* cost, int radius);
			int FindDirectCostToPointOnRadius(void* startNode, void* endNode, float* cost, int radius);

		private:
			void GoalReached(PathNode* node, void* start, void* end, std::vector<void*> *path);
			float CheckSafety(PathNode* node);
			float LeastCostEstimateLocal(int nodeStartIndex);
			static inline float DiagonalDistance(int xStart, int yStart, int xEnd, int yEnd);
			void FixStartEndNode(void** startNode, void** endNode);
			void FixNode(void** Node);

			// allocates the node array, don't call more than once
			PathNode* AllocatePathNode();

			const unsigned ALLOCATE;		// how big a block of pathnodes to allocate at once
			const unsigned BLOCKSIZE;		// how many useable pathnodes are in a block

			Graph* graph;
			PathNode* pathNodeMem;			// pointer to root of PathNode blocks
			PathNode* pathNodeMemForFree;	// pointer to root of PathNode blocks
			PathNode** heapArrayMem;		// pointer to root of a PathNode pointer array

			unsigned availMem;				// # PathNodes available in the current block
			unsigned pathNodeCount;			// the # of PathNodes in use
			unsigned frame;					// incremented with every solve, used to determine if cached data needs to be refreshed
			unsigned checksum;				// the checksum of the last successful "Solve".
	};
}


#endif
