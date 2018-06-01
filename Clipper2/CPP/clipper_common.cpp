#include "clipper_common.h"
#include <ostream>

namespace clipperlib {
	std::ostream& operator <<(std::ostream &s, const Point64 &pt)
	{
		s << pt.x << "," << pt.y << " ";
		return s;
	}

	std::ostream& operator <<(std::ostream &s, const Path &path)
	{
		if (path.empty()) return s;
		Path::size_type last = path.size() - 1;
		for (Path::size_type i = 0; i < last; i++) s << path[i] << " ";
		s << path[last] << "\n";
		return s;
	}

	std::ostream& operator <<(std::ostream &s, const Paths &paths)
	{
		for (Paths::size_type i = 0; i < paths.size(); i++) s << paths[i];
		s << "\n";
		return s;
	}
} // namespace clipperlib
