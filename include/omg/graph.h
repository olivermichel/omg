
#ifndef OMG_GRAPH_H
#define OMG_GRAPH_H

#include <iostream>
#include <map>
#include <vector>
#include <numeric>

namespace omg
{
	template<class VertexType, class EdgeType>
	class Graph
	{
	public:

		static const std::size_t ELEMENT_INDEX_INIT = 1;

		class exception : public std::exception
		{
		public:

			explicit exception(const char* what)
				: _what(what)
			{ }

			explicit exception(const std::string& what)
				: _what(what)
			{ }

			virtual const char* what() const throw()
			{
				return _what.c_str();
			}

			virtual ~exception() throw()
			{ }

		private:
			std::string _what;
		};

		template<class T>
		class container
		{
			friend class Graph<VertexType, EdgeType>;
		public:

			container() = delete;

			container(T e)
				: _element(e)
			{ }

		protected:
			T _element;
		};

		// forward declare edge_container for referenceability in vertex_container
		template<class> class edge_container;

		template<class>
		class vertex_container : public container<VertexType>
		{
			friend class Graph<VertexType, EdgeType>;
			friend class vertex_proxy;

		public:

			vertex_container(VertexType e)
				: container<VertexType>(e),
				  _neighbors(),
				  _in_edges(),
				  _out_edges(),
				  _super_vertex(),
				  _sub_vertices()
			{ }

		private:

			void _add_neighbor(vertex_container<VertexType>* n)
			{
				_neighbors.push_back(n);
			}

			void _remove_neighbor(vertex_container<VertexType>* n)
			{
				_neighbors.erase(std::find(_neighbors.begin(), _neighbors.end(), n));
			}

			void _add_in_edge(edge_container<EdgeType>* e)
			{
				_in_edges.push_back(e);
			}

			void _remove_in_edge(edge_container<EdgeType>* e)
			{
				_in_edges.erase(std::find(_in_edges.begin(), _in_edges.end(), e));
			}

			void _add_out_edge(edge_container<EdgeType>* e)
			{
				_out_edges.push_back(e);
			}

			void _remove_out_edge(edge_container<EdgeType>* e)
			{
				_out_edges.erase(std::find(_out_edges.begin(), _out_edges.end(), e));
			}

			void _add_sub_vertex(vertex_container<VertexType>* v)
			{
				_sub_vertices.push_back(v);
			}

			void _remove_sub_vertex(vertex_container<VertexType>* v)
			{
				_sub_vertices.erase(std::find(_sub_vertices.begin(), _sub_vertices.end(), v));
			}

			void _set_super_vertex(vertex_container<VertexType>* v)
			{
				_super_vertex = v;
			}

			void _unset_super_vertex()
			{
				_super_vertex = nullptr;
			}

			std::vector<vertex_container<VertexType>*> _neighbors;
			std::vector<edge_container<EdgeType>*>     _in_edges;
			std::vector<edge_container<EdgeType>*>     _out_edges;
			vertex_container<VertexType>*              _super_vertex;
			std::vector<vertex_container<VertexType>*> _sub_vertices;
		};

		template<class>
		class edge_container : public container<EdgeType>
		{
			friend class Graph<VertexType, EdgeType>;
			friend class edge_proxy;

		public:

			edge_container(EdgeType e)
				: container<EdgeType>(e),
				  _from(nullptr),
				  _to(nullptr)
			{ }

		private:

			inline void _set_from(vertex_container<VertexType>* v)
			{
				_from = v;
			}

			inline void _set_to(vertex_container<VertexType>* v)
			{
				_to = v;
			}

			inline void _unset_from()
			{
				_from = nullptr;
			}

			inline void _unset_to()
			{
				_to = nullptr;
			}

			void _add_sub_edge(edge_container<EdgeType>* e)
			{
				_sub_edges.push_back(e);
			}

			void _remove_sub_edge(edge_container<EdgeType>* e)
			{
				_sub_edges.erase(std::find(_sub_edges.begin(), _sub_edges.end(), e));
			}

			void _set_super_edge(edge_container<EdgeType>* e)
			{
				_super_edge = {e};
			}

			void _unset_super_edge()
			{
				_super_edge.clear();
			}

			void _add_super_edge(edge_container<EdgeType>* e)
			{
				_super_edge.push_back(e);
			}

			vertex_container<VertexType>* _from;

			vertex_container<VertexType>* _to;

			// sub edge may be mapped to a series of super edges
			std::vector<edge_container<EdgeType>*> _super_edge;

			std::vector<edge_container<EdgeType>*> _sub_edges;
		};

		class vertex_proxy
		{
			friend class Graph<VertexType, EdgeType>;
		public:

			class iterator
			{
				friend class vertex_proxy;

			public:

				iterator() = delete;

				iterator(typename std::map<std::size_t, vertex_container<VertexType>>::iterator i)
					: _i(i)
				{ }

				inline iterator operator++()
				{
					return ++_i;
				}

				inline iterator operator++(int i)
				{
					return _i++;
				}

				inline VertexType& operator*()
				{
					return _i->second._element;
				}

				inline bool operator==(const iterator& other)
				{
					return _i == other._i;
				}

				inline bool operator!=(const iterator& other)
				{
					return _i != other._i;
				}

				inline std::size_t id()
				{
					return _i->first;
				}

				bool has_neighbors()
				{
					return !(_i->second._neighbors.empty());
				}

				void map(iterator other)
				{
					_i->second._add_sub_vertex(&(other._container()));
					other._container()._set_super_vertex(&(this->_container()));
				}

				template<typename F>
				void map(iterator other, F f)
				{
					map(other);
					f(**this, *other);
				}

				void unmap(iterator other)
				{
					_i->second._remove_sub_vertex(&(other._container()));
					other._container()._unset_super_vertex();
				}

				template<typename F>
				void unmap(iterator other, F f)
				{
					unmap(other);
					f(**this, *other);
				}

				bool has_subvertices()
				{
					return !_container()._sub_vertices.empty();
				}

				bool has_supervertex()
				{
					return _container()._super_vertex != nullptr;
				}

				inline VertexType* super_vertex()
				{
					return has_supervertex() ? &(_container()._super_vertex->_element) : nullptr;
				}

				inline std::vector<VertexType*> sub_vertices()
				{
					std::vector<VertexType*> v;

					for(auto c : _container()._sub_vertices)
						v.push_back(&(c->_element));

					return v;
				}

				inline vertex_container<VertexType>& _container()
				{
					return _i->second;
				}

				inline VertexType* _ptr()
				{
					return &(_i->second._element);
				}

			private:

				typename std::map<std::size_t, vertex_container<VertexType>>::iterator _i;
			};

			vertex_proxy() = delete;

			explicit vertex_proxy(Graph<VertexType, EdgeType>* graph)
				: _graph(graph),
				  _vertices(),
				  _i(Graph::ELEMENT_INDEX_INIT)
			{ }

			iterator add(const VertexType& vertex)
			{
				return iterator(_vertices.insert(_vertices.end(), std::make_pair(_i++, vertex)));
			}

			iterator remove(iterator pos)
			{
				if(pos.has_neighbors()) {
					throw exception("vertex_proxy::remove(): vertex still has neighbors.");
				}

				return iterator(_vertices.erase(pos._i));
			}

			iterator operator[](std::size_t index) throw(exception)
			{
				auto i = _vertices.find(index);

				if (i != _vertices.end())
					return iterator(_vertices.find(index));
				else
					throw exception("vertex_proxy::operator[]: index does not exist.");
			}

			inline iterator begin()
			{
				return iterator(_vertices.begin());
			}

			inline iterator end()
			{
				return iterator(_vertices.end());
			}

			inline std::size_t count()
			{
				return _vertices.size();
			}

		private:

			void _add_neighbor(iterator neighbor, iterator vertex) // invoked from edge_proxy
			{
				vertex._container()._add_neighbor(&neighbor._container());
			}

			Graph<VertexType, EdgeType>* _graph;

			std::size_t _i;

			std::map<std::size_t, vertex_container<VertexType>> _vertices;
		};

		class edge_proxy
		{
			friend class Graph<VertexType, EdgeType>;
			friend class vertex_proxy;

		public:
			class iterator
			{
				friend class edge_proxy;
				friend class vertex_proxy;

			public:

				iterator() = delete;

				inline iterator(typename std::map<std::size_t, edge_container<EdgeType>>::iterator i)
					: _i(i) { }

				inline iterator operator++()
				{
					return ++_i;
				}

				inline iterator operator++(int i)
				{
					return _i++;
				}

				inline EdgeType& operator*()
				{
					return _i->second._element;
				}

				inline bool operator==(const iterator& other)
				{
					return _i == other._i;
				}

				inline bool operator!=(const iterator& other)
				{
					return _i != other._i;
				}

				inline std::size_t id()
				{
					return _i->first;
				}

				void map_link(iterator other)
				{
					_i->second._add_sub_edge(&(other._container()));
					other._container()._set_super_edge(&(this->_container()));
				}

				template<typename F>
				void map_link(iterator other, F f)
				{
					map_link(other);
					f(**this, *other);
				}

				void map_path(iterator other)
				{
					_i->second._add_sub_edge(&(other._container()));
					other._container()._add_super_edge(&(this->_container()));
				}

				template<typename F>
				void map_path(iterator other, F f)
				{
					map_path(other);
					f(**this, *other);
				}

				void unmap(iterator other)
				{
					_i->second._remove_sub_edge(&(other._container()));
					other._container()._unset_super_edge();
				}

				template<typename F>
				void unmap(iterator other, F f)
				{
					unmap(other);
					f(**this, *other);
				}

				bool has_subedges()
				{
					return !_container()._sub_edges.empty();
				}

				bool has_superedge()
				{
					return !_container()._super_edge.empty();
				}

				inline std::vector<EdgeType*> super_edge()
				{
					std::vector<EdgeType*> v;

					for(auto c : _container()._super_edge)
						v.push_back(&(c->_element));

					return v;
				}

				inline std::vector<EdgeType*> sub_edges()
				{
					std::vector<EdgeType*> v;

					for(auto c : _container()._sub_edges)
						v.push_back(&(c->_element));

					return v;
				}

				inline edge_container<EdgeType>& _container()
				{
					return _i->second;
				}

				inline EdgeType* _ptr()
				{
					return &(_i->second._element);
				}

			private:

				typename std::map<std::size_t, edge_container<EdgeType>>::iterator _i;
			};

			edge_proxy() = delete;

			edge_proxy(Graph<VertexType, EdgeType>* graph)
				: _graph(graph),
				  _edges(),
				  _i(Graph::ELEMENT_INDEX_INIT)
			{ }

			iterator add(
				typename vertex_proxy::iterator from,
				typename vertex_proxy::iterator to,
				const EdgeType& edge)
			{
				auto i = iterator(_edges.insert(_edges.end(), std::make_pair(_i++, edge)));
				_connect(from, to, i);
				_set_neighbors_bidirectional(from, to);
				_set_edges_bidirectional(from, to, i);
				return i;
			}

			iterator remove(iterator edge_it)
			{
				_unset_neighbors_bidirectional(edge_it);
				_unset_edges_bidirectional(edge_it);
				return iterator(_edges.erase(edge_it._i));
			}

			iterator operator[](std::size_t index) throw(exception)
			{
				auto i = _edges.find(index);

				if (i != _edges.end())
					return iterator(_edges.find(index));
				else
					throw exception("edge_proxy::operator[]: index does not exist.");
			}

			iterator begin()
			{
				return iterator(_edges.begin());
			}

			iterator end()
			{
				return iterator(_edges.end());
			}

			std::size_t count()
			{
				return _edges.size();
			}

		private:

			void _connect(
				typename vertex_proxy::iterator from,
				typename vertex_proxy::iterator to,
				iterator edge)
			{
				edge._container()._set_from(&from._container());
				edge._container()._set_to(&to._container());
			}

			void _set_neighbors_bidirectional(
				typename vertex_proxy::iterator from,
				typename vertex_proxy::iterator to)
			{
				_graph->vertices._add_neighbor(from, to);
				_graph->vertices._add_neighbor(to, from);
			}

			void _unset_neighbors_bidirectional(iterator edge)
			{
				edge._container()._from->_remove_neighbor(edge._container()._to);
				edge._container()._to->_remove_neighbor(edge._container()._from);
			}

			void _set_edges_bidirectional(
				typename vertex_proxy::iterator from,
				typename vertex_proxy::iterator to,
				iterator edge)
			{
				from._container()._add_out_edge(&edge._container());
				to._container()._add_in_edge(&edge._container());
				to._container()._add_out_edge(&edge._container());
				from._container()._add_in_edge(&edge._container());
			}

			void _unset_edges_bidirectional(iterator edge)
			{
				edge._container()._from->_remove_in_edge(&edge._container());
				edge._container()._to->_remove_in_edge(&edge._container());
				edge._container()._from->_remove_out_edge(&edge._container());
				edge._container()._to->_remove_out_edge(&edge._container());
			}

			Graph<VertexType, EdgeType>* _graph;
			std::size_t _i;
			std::map<std::size_t, edge_container<EdgeType>> _edges;
		};

	public:

		Graph()
			: vertices(this),
			  edges(this),
			  _supergraph(nullptr),
			  _subgraphs()
		{ }

		void map(Graph<VertexType, EdgeType>* subgraph)
		{
			_subgraphs.push_back(subgraph);
			subgraph->_supergraph = this;
		}

		bool has_subgraph(Graph<VertexType, EdgeType>* subgraph) const
		{
			return std::find(_subgraphs.begin(), _subgraphs.end(), subgraph) != _subgraphs.end();
		}

		bool has_subgraphs() const
		{
			return !_subgraphs.empty();
		}

		void unmap(Graph<VertexType, EdgeType>* subgraph)
		{
			_subgraphs.erase(std::find(_subgraphs.begin(), _subgraphs.end(), subgraph));
			subgraph->_supergraph = nullptr;
		}

		const Graph<VertexType, EdgeType>* supergraph()
		{
			return _supergraph;
		};

		const std::vector<Graph<VertexType, EdgeType>*>& subgraphs()
		{
			return _subgraphs;
		};

		template<class T, class U> // T, U in order not to shadow V, E
		friend std::ostream& operator<<(std::ostream& os, Graph<T, U>& g);

		vertex_proxy vertices;

		edge_proxy edges;

	private:

		void _print_adjacency_list(std::ostream& os)
		{
			for(std::pair<std::size_t, vertex_container<VertexType>> p : vertices._vertices) {
				os << " " << p.second._element << " -> [ ";
				for(unsigned i = 0; i < p.second._neighbors.size(); i++) {
					os << p.second._neighbors[i]->_element
					<< (i == p.second._neighbors.size()-2 ? ", " : " ");
				}
				os << "]" << std::endl;
			}
		}

		Graph<VertexType, EdgeType>* _supergraph;

		std::vector<Graph<VertexType, EdgeType>*> _subgraphs;
	};

	template<class VertexType, class EdgeType>
	std::ostream& operator<<(std::ostream& os, Graph<VertexType, EdgeType>& g)
	{
		os << "Graph(n=" << g.vertices.count() << ", m=" << g.edges.count() << ")"
		<<  std::endl;
		g._print_adjacency_list(os);
		return os;
	};

}

#endif
