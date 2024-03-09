package diffmap

import (
	"github.com/google/syzkaller/pkg/bitmap"
)

var (
	diff_BB_coverage_ratio     uint64 = 2
	diff_BB_coverage_threshold        = 100
)

type DiffMap struct {
	data      *bitmap.Bitmap
	hit_count *map[uint32]int
}

func Init() *DiffMap {
	bitmap := bitmap.New(diff_region_end-diff_region_start, diff_region_start)
	for _, region := range diff_regions {
		bitmap.SetRegion(region[0], region[1])
	}
	return &DiffMap{
		data:      bitmap,
		hit_count: &map[uint32]int{},
	}
}

func (dm *DiffMap) CountDiffPrio(cover []uint32) (prio int64) {
	for _, pc := range cover {
		if dm.data.Test(pc) {
			(*dm.hit_count)[pc]++
			// TODO: priority algorithm pending improvement
			if (*dm.hit_count)[pc] < diff_BB_coverage_threshold {
				prio += int64(diff_BB_coverage_ratio)
			}
		}
	}
	return
}
