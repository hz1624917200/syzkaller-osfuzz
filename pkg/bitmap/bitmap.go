package bitmap

import "github.com/google/syzkaller/pkg/log"

type Bitmap struct {
	data   []byte
	size   uint32
	offset uint32 // the offset of the first bit
}

func New(size uint32, offset uint32) *Bitmap {
	return &Bitmap{
		data:   make([]byte, (size+7)/8),
		size:   size,
		offset: offset,
	}
}

func (b *Bitmap) SetBit(pos uint32) {
	if pos < b.offset || pos >= b.offset+b.size {
		return
	}
	pos -= b.offset
	b.data[pos/8] |= 1 << uint(pos%8)
}

func (b *Bitmap) SetRegion(start uint32, end uint32) {
	if start >= end {
		return
	}
	if start < b.offset {
		start = b.offset
	}
	if end >= b.offset+b.size {
		end = b.offset + b.size
	}

	start -= b.offset
	end -= b.offset

	if end-start > 8 {
		start_index := start / 8
		end_index := (end - 1) / 8
		for i := start; i < ((start_index + 1) >> 3); i++ {
			b.SetBit(i + b.offset)
		}
		for i := end_index << 3; i < end; i++ {
			b.SetBit(i + b.offset)
		}
		for i := start_index + 1; i < end_index; i++ {
			b.data[i] = 0xff
		}
	} else {
		for i := start; i < end; i++ {
			b.data[i/8] |= 1 << uint(i%8)
		}
	}
}

func (b *Bitmap) Test(pos uint32) bool {
	if pos < 0x81000000 {
		log.Logf(0, "Error pos < 0x81000000: %v\n", pos)
		panic("Error pos < 0x81000000\n")
	}
	if pos < b.offset || pos >= b.offset+b.size {
		return false
	}
	pos -= b.offset
	return b.data[pos/8]&(1<<uint(pos%8)) != 0
}

func (b *Bitmap) Size() uint32 {
	return b.size
}

func (b *Bitmap) Offset() uint32 {
	return b.offset
}
