/*******************************************************************************
* Author    :  Angus Johnson                                                   *
* Version   :  10.0 (beta)                                                     *
* Date      :  8 Noveber 2017                                                  *
* Website   :  http://www.angusj.com                                           *
* Copyright :  Angus Johnson 2010-2017                                         *
* Purpose   :  Base clipping module                                            *
* License   : http://www.boost.org/LICENSE_1_0.txt                             *
*******************************************************************************/

#ifndef clipper_common_h
#define clipper_common_h

#include <vector>
#include <stdint.h>
#include <ostream>

namespace clipperlib {

	struct Point64 {
		int64_t x;
		int64_t y;
		Point64(int64_t x = 0, int64_t y = 0) : x(x), y(y) {};

		friend inline bool operator== (const Point64 &a, const Point64 &b)
		{
			return a.x == b.x && a.y == b.y;
		}
		friend inline bool operator!= (const Point64 &a, const Point64 &b)
		{
			return a.x != b.x || a.y != b.y;
		}
	};

	typedef std::vector< Point64 > Path;
	typedef std::vector< Path > Paths;

	inline Path& operator <<(Path &path, const Point64 &pt) { path.push_back(pt); return path; }
	inline Paths& operator <<(Paths &paths, const Path &path) { paths.push_back(path); return paths; }

	std::ostream& operator <<(std::ostream &s, const Point64 &p);
	std::ostream& operator <<(std::ostream &s, const Path &p);
	std::ostream& operator <<(std::ostream &s, const Paths &p);

} // namespace clipperlib

#endif // clipper_common_h
