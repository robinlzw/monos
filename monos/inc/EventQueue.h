#pragma once

#include <stddef.h>

#include "cgTypes.h"
#include "CollapseSpec.h"
#include "Heap.h"

class HeapEvent {
public:
	const Event * const e;
	HeapEvent(const Event * p_t);

	const NT& time() const { return e->eventTime; };
	friend std::ostream& operator<<(std::ostream& os, const Event& e);
public:
	CGAL::Comparison_result compare(const HeapEvent &o) const {
			 if (this->time() < o.time()) {
				 return CGAL::SMALLER;
			 } else if (this->time() > o.time()) {
				 return CGAL::LARGER;
			 } else {
				 return CGAL::EQUAL;
			 }
		 }
public:
	 bool operator< (const HeapEvent &o) const { return compare(o) == CGAL::SMALLER; }
	 bool operator> (const HeapEvent &o) const { return compare(o) == CGAL::LARGER; }
	 bool operator>= (const HeapEvent &o) const { return !(*this < o); };
	 bool operator<= (const HeapEvent &o) const { return !(*this > o); };
	 bool operator== (const HeapEvent &o) const { return compare(o) == CGAL::EQUAL; }
	 bool operator!= (const HeapEvent &o) const { return !(*this == o); };
};

class EventQueueItem : public HeapItemBase <HeapEvent> {
private:
public:
	EventQueueItem(const Event * e)
	: HeapItemBase<HeapEvent>(HeapEvent(e))
	{};
};

class EventQueue :  public HeapBase <HeapEvent, EventQueueItem> {
private:
	using Base = HeapBase <HeapEvent, EventQueueItem>;

private:
	std::vector<const Event *> need_update;
	std::vector<const Event *> need_dropping;
	FixedVector<bool> tidx_in_need_dropping;
	FixedVector<bool> tidx_in_need_update;

	FixedVector<ElementType> tidx_to_qitem_map;

	void tidx_to_qitem_map_add(const Event * t, ElementType qi);
	void drop_by_tidx(unsigned tidx);
	void assert_no_pending() const;
public:
	EventQueue(const Events& events, const Chain& chain);

	void update_by_tidx(unsigned tidx);
	/* we /could/ make this const, and our heap a mutable
	 * object attribute and the vectors also mutable.
	 * At which point pretty much everything in here is
	 * declared mutable, so let's just not.
	 */
	const ElementType& peak() const {
		assert_no_pending();
		return Base::peak();
	}
	const ElementType& peak(int idx) const {
		assert_no_pending();
		return Base::peak(idx);
	}
	using Base::size;
	using Base::empty;

	void process_pending_updates();

	void needs_update(const Event * t, bool may_have_valid_collapse_spec = false);
	void needs_dropping(Event * t);

	bool in_needs_update(const ul edgeIdx) const;
	bool in_needs_dropping(const Event * t) const;

	bool is_valid_heap() const;
};