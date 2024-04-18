package diffmap

import (
	"sort"

	"github.com/google/syzkaller/pkg/bitmap"
)

var (
	diff_BB_coverage_ratio       int64 = 1
	diff_BB_coverage_threshold         = 100
	diff_func_coverage_ratio     int64 = 2
	diff_func_coverage_threshold       = 1000
)

type DiffMap struct {
	BB_map        *bitmap.Bitmap
	BB_hitcount   *map[uint32]int
	func_hitcount *map[uint32]int
}

func Init() *DiffMap {
	bitmap := bitmap.New(diff_region_end-diff_region_start, diff_region_start)
	for _, region := range diff_BB_regions {
		bitmap.SetRegion(region[0], region[1])
	}
	return &DiffMap{
		BB_map:      bitmap,
		BB_hitcount: &map[uint32]int{},
	}
}

func (dm *DiffMap) CountDiffPrio(cover []uint32) (prio int64) {
	for _, pc := range cover {
		if dm.BB_map.Test(pc) { // Test BB Coverage
			(*dm.BB_hitcount)[pc]++
			// priority algorithm params can be improved?
			if (*dm.BB_hitcount)[pc] < diff_BB_coverage_threshold {
				prio += diff_BB_coverage_ratio
			}
		}
		if dm.FuncDiffMatch(pc) { // Test Function Coverage
			prio += diff_func_coverage_ratio
		}
	}
	return
}

func (dm *DiffMap) FuncDiffMatch(addr uint32) bool {
	index := sort.Search(len(diff_func_regions), func(i int) bool { return diff_func_regions[i][0] >= addr })
	if index == 0 {
		return false
	}
	if diff_func_regions[index-1][1] >= addr {
		(*dm.func_hitcount)[addr]++
		if (*dm.func_hitcount)[addr] <= diff_func_coverage_threshold {
			return true
		}
	}
	return false
}
