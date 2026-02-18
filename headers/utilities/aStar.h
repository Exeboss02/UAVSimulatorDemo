#pragma once

#include <memory>
#include <vector>
#include <stdexcept>
#include <algorithm>

#include "utilities/logger.h"
#include "utilities/minPriorityQueue.h"
#include "gameObjects/gameObject3D.h"

namespace DX = DirectX;

/// <summary>
/// A path between two vertices
/// </summary>
/// <param name="from">Starting vertex of the edge</param>
/// <param name="to">Ending vertex of the edge</param>
/// <param name="cost">Cost to traverse this edge</param>
struct Edge {
	struct AStarVertex* from;
	struct AStarVertex* to;
	unsigned int cost;
	Edge(struct AStarVertex* from, struct AStarVertex* to, unsigned int cost) : from(from), to(to), cost(cost) {}
};

/// <summary>
/// A vertex represents a point in the graph with its position, edges to neighboring vertexs, and costs for pathfinding.
/// </summary>
/// <param name="position">( x, y ) coordinate of vertex</param>
/// <param name="edges">List of edges connecting this vertex to its neighbors</param>
/// <param name="gCost">Cost from the start vertex to this vertex</param>
/// <param name="hCost">Heuristic cost from this vertex to the goal vertex</param>
/// <param name="fCost">Total cost (gCost + hCost) used for prioritization in the open list</param>
/// <param name="parent">Pointer to the parent vertex in the path, used for path reconstruction</param>
struct AStarVertex : public GameObject3D {
	std::vector<struct Edge> edges;
	int gCost, hCost, fCost;
	std::shared_ptr<AStarVertex> parent;
	
	AStarVertex() : gCost(INT_MAX), hCost(0), fCost(0), parent(nullptr) {}
	
	void Initialize(DX::XMVECTOR pos) { this->transform.SetPosition(pos); }
	
	inline bool operator<(const AStarVertex& other) const { return this->fCost < other.fCost; }
	inline bool operator==(const AStarVertex& other) const { return DX::XMVector4NearEqual(this->GetGlobalPosition(), other.GetGlobalPosition(), DX::XMVectorSet(0.1f, 0.1f, 0.1f, 0.1f)); }
};

class AStar {
public:
	AStar() = default;
	AStar(AStar& other) = delete;
	AStar& operator=(const AStar&) = delete;

	~AStar() = default;

	bool AddVertex(std::shared_ptr<AStarVertex> vertex);
	void RemoveVertex(std::shared_ptr<AStarVertex> vertexToRemove);

	bool AddEdge(std::shared_ptr<AStarVertex> from, std::shared_ptr<AStarVertex> to, const unsigned int cost);

	void SetGoal(std::shared_ptr<AStarVertex> goal);

	std::vector<std::shared_ptr<AStarVertex>> FindPath(std::shared_ptr<AStarVertex> start);

	void PrintAllEdges();

private:
	std::vector<std::shared_ptr<AStarVertex>> vertices;
	std::shared_ptr<AStarVertex> goal;

	MinPriorityQueue<AStarVertex*> openList;
	std::vector<AStarVertex*> closedList;

	std::vector<std::shared_ptr<AStarVertex>> path;

	const int CalculateHeuristic(AStarVertex* vertex);
	void ClearPathfindingData();

	std::shared_ptr<AStarVertex> GetVertex(DX::XMVECTOR position);

	const int GetEdgeCost(AStarVertex* from, AStarVertex* to);
	std::vector<AStarVertex*> GetNeighbors(AStarVertex* vertex);
	std::vector<std::shared_ptr<AStarVertex>>& ReconstructPath(std::shared_ptr<AStarVertex> goalVertex);
};


/// <summary>
/// Creates a new vertex at the specified (x, y) position.
/// </summary>
/// <param name="position">Integer pair representing the coordinates of the vertex.</param>
/// <returns>Returns true: Successfully added vertex. Returns false: vertex already exsist</returns>
inline bool AStar::AddVertex(std::shared_ptr<AStarVertex> vertex) {
	DX::XMVECTOR position = vertex->GetGlobalPosition();
	if (this->GetVertex(position)) {
		Logger::Log("vertex already exists");
		return false;
	}

	this->vertices.push_back(vertex);
	return true;
}

/// <summary>
/// Removes a vertex at the specified position from the graph, along with all edges connected to it.
/// </summary>
/// <param name="position">The position of the vertex to remove from the graph.</param>
inline void AStar::RemoveVertex(std::shared_ptr<AStarVertex> vertexToRemove) {
	// Remove edges connected to this vertex
	for (auto& vertex : this->vertices) {
		for (auto& edge : vertex->edges) {
			if (*edge.to == *vertexToRemove) {
				vertex->edges.erase(std::remove_if(vertex->edges.begin(), vertex->edges.end(),
												   [vertexToRemove](const Edge& edge) { return *edge.to == *vertexToRemove.get(); }),
									vertex->edges.end());
			}
		}
	}

	// Remove the vertex itself
	this->vertices.erase(std::remove_if(this->vertices.begin(), this->vertices.end(),
										[vertexToRemove](const std::shared_ptr<AStarVertex>& vertex) {
											return *vertex.get() == *vertexToRemove.get();
										}),
						 this->vertices.end());
}

/// <summary>
/// Adds a non-directional edge between two vertices with the specified cost.
/// </summary>
/// <param name="from"></param>
/// <param name="to"></param>
/// <param name="cost"></param>
/// <returns>Successfully added edge</returns>
inline bool AStar::AddEdge(std::shared_ptr<AStarVertex> from, std::shared_ptr<AStarVertex> to, const unsigned int cost) {
	
	if (!from || !to) {
		Logger::Log("Invalid vertexs for edge creation.");
		return false;
	}
	
	// Check if the edge already exists
	for (auto& edge : from->edges) {
		if (*edge.to == *to) {
			//Logger::Log("Edge already exists between (", from->GetGlobalPosition(), ") and (", to->GetGlobalPosition(), ").");
			return false; // Edge already exists
		}
	}
	from->edges.emplace_back(from.get(), to.get(), cost);
	to->edges.emplace_back(to.get(), from.get(), cost);
	return true;
}

inline void AStar::SetGoal(std::shared_ptr<AStarVertex> goal) { this->goal = goal; }

/// <summary>
/// Finds the shortest path between two vertices using the A* pathfinding algorithm.
/// </summary>
/// <param name="start">A shared pointer to the starting vertex for the pathfinding search.</param>
/// <param name="goal">A shared pointer to the target vertex to reach.</param>
/// <returns>A vector of coordinate pairs representing the path from start to goal.</returns>
inline std::vector<std::shared_ptr<AStarVertex>> AStar::FindPath(std::shared_ptr<AStarVertex> start) {
	this->ClearPathfindingData(); // Reset the state of the algorithm before starting a new search
	
	if (!this->goal) {
	    Logger::Log("NO GOAL HAS BEEN SET!");
	    return {};
	}
	
	// Get the actual start vertex from the vertices vector
	std::shared_ptr<AStarVertex> startVertex = this->GetVertex(start->GetGlobalPosition());
	if (!startVertex) {
		Logger::Log("Start vertex not found in graph!");
		return {};
	}
	
	startVertex->gCost = 0;
	startVertex->hCost = this->CalculateHeuristic(startVertex.get());
	startVertex->fCost = startVertex->gCost + startVertex->hCost;
	this->openList.Enqueue(startVertex.get(), startVertex->fCost);

	while (!this->openList.IsEmpty()) {
		AStarVertex* current = this->openList.Dequeue();

		if (*current == *this->goal.get()) { // If goal is reached, reconstruct path
			std::shared_ptr<AStarVertex> goalVertex = this->GetVertex(current->GetGlobalPosition());
			return this->ReconstructPath(goalVertex);
		}

		this->closedList.push_back(current); // Mark current vertex as visited by adding it to the closed list
		
		std::shared_ptr<AStarVertex> currentSharedPtr = this->GetVertex(current->GetGlobalPosition());
		
		for (auto& neighbor : this->GetNeighbors(current)) {
			if (std::find(closedList.begin(), closedList.end(), neighbor) != closedList.end()) {
				continue; // Skip if neighbor is in closed list
			}
			
			int tentativeGCost = current->gCost + this->GetEdgeCost(current, neighbor);

			// If neighbor is already in open list with better cost, skip it
			// Only update if this path is better
			if (tentativeGCost < neighbor->gCost) {
				neighbor->gCost = tentativeGCost;
				neighbor->hCost = this->CalculateHeuristic(neighbor);
				neighbor->fCost = neighbor->gCost + neighbor->hCost;
				neighbor->parent = currentSharedPtr;  // Use the shared_ptr we already retrieved
				this->openList.Enqueue(neighbor, neighbor->fCost);
			}
		}
	}

	return {}; // Return empty path if no path is found
}

inline void AStar::PrintAllEdges() {
	for (const auto& vertex : this->vertices) {
		for (const auto& edge : vertex->edges) {
			Logger::Log("Edge from (", DX::XMVectorGetX(vertex->GetGlobalPosition()), ", ", DX::XMVectorGetZ(vertex->GetGlobalPosition()), ") to (",
						DX::XMVectorGetX(edge.to->GetGlobalPosition()), ", ", DX::XMVectorGetZ(edge.to->GetGlobalPosition()), ") with cost ", edge.cost);
		}
	}
}

/// <summary>
/// Calculates the heuristic cost (hCost) between the current vertex and the goal vertex using the Euclidean distance.
/// </summary>
/// <param name="vertex">Current vertex we want to calculate heuristic for.</param>
/// <param name="goal">Goal vertex used for calculating the heuristic.</param>
/// <returns>The heuristic value in form of an integer.</returns>
inline const int AStar::CalculateHeuristic(AStarVertex* vertex) {

	DX::XMVECTOR goalPos = this->goal->GetGlobalPosition();
	DX::XMVECTOR vertexPos = vertex->GetGlobalPosition();

	DX::XMVECTOR delta = DX::XMVectorSubtract(vertexPos, goalPos);
	float distance = DX::XMVectorGetX(DX::XMVector3Length(delta));

	return static_cast<int>(distance);
}

/// <summary>
/// Reset the state of the A* algorithm for a new interation.
/// </summary>
inline void AStar::ClearPathfindingData() {
	// Reset state
	this->openList.Clear();
	this->closedList.clear();

	// Reset all vertices
	for (auto& vertex : this->vertices) {
		vertex->gCost = INT_MAX;
		vertex->hCost = 0;
		vertex->fCost = 0;
		vertex->parent = nullptr;
	}
}

/// <summary>
/// Retrieves a vertex from the collection by its position.
/// </summary>
/// <param name="position">The position of the vertex to find.</param>
/// <returns>A shared pointer to the vertex, or nullptr if no matching vertex is found.</returns>
inline std::shared_ptr<AStarVertex> AStar::GetVertex(DX::XMVECTOR position) {
	for (auto& vertex : this->vertices) {
		if (DX::XMVector4NearEqual(vertex->GetGlobalPosition(), position, DX::XMVectorSet(0.1f, 0.1f, 0.1f, 0.1f))) {
			return vertex;
		}
	}
	return nullptr; // vertex not found
}

/// <summary>
/// Gets the cost of the edge between two specified vertices.
/// </summary>
/// <param name="from">A pointer to the source vertex of the edge.</param>
/// <param name="to">A pointer to the destination vertex of the edge.</param>
/// <returns>The cost of the edge.</returns>
inline const int AStar::GetEdgeCost(AStarVertex* from, AStarVertex* to) { 
	for (auto& edge : from->edges) {
		if (*edge.to == *to) {
			return edge.cost;
		}
	}
	throw std::runtime_error("Edge does not exist between the specified vertices.");
}

/// <summary>
/// Retrieves all neighboring vertices of a given vertex.
/// </summary>
/// <param name="vertex">The vertex whose neighbors should be retrieved.</param>
/// <returns>A vector containing shared pointers to all neighboring vertices.</returns>
inline std::vector<AStarVertex*> AStar::GetNeighbors(AStarVertex* vertex) { 
	std::vector<AStarVertex*> neighbors;
	for (auto& edge : vertex->edges) {
		neighbors.push_back(edge.to);
	}
	return neighbors;
}

/// <summary>
/// Reconstructs the path from the start to the goal by following parent pointers backward from the goal vertex.
/// </summary>
/// <returns>A vector of coordinate pairs representing the path from start to goal.</returns>
inline std::vector<std::shared_ptr<AStarVertex>>& AStar::ReconstructPath(std::shared_ptr<AStarVertex> goalVertex) {
	this->path.clear();

	std::shared_ptr<AStarVertex> currentvertex = goalVertex;

	while (currentvertex) {
		this->path.emplace_back(currentvertex);
		currentvertex = currentvertex->parent;
	}

	std::reverse(this->path.begin(), this->path.end());

	Logger::Log("Path found with ", this->path.size(), " vertices.");
	for (int i = 0; i < this->path.size(); ++i) {
		Logger::Log("Vertex ", i, ": (", DX::XMVectorGetX(this->path[i]->GetGlobalPosition()), ", ", DX::XMVectorGetZ(this->path[i]->GetGlobalPosition()), ")");
	}

	//this->PrintAllEdges();

	return this->path;
}
