// List template 

// Copyright (C) 2008 Joseph H. Allen
// See file COPYING for license.

template<class T>
class Dlist
  {
  public:

  struct Entry
    {
    T val;
    Entry *dnext;
    Entry *dprev;
    Entry()
      {
      dnext = 0;
      dprev = 0;
      }
    };
  Entry *dfirst; // First item
  Entry *dlast; // Last item
  int dlen; // No. items in list

  // Return number of items in list
  int len()
    {
    return dlen;
    }

  // Remember last array reference
  Entry *last_ref;
  int last_idx;

  // We automatically cast to this for type-safe (ptr==NULL) tests.
  typedef Entry *Entrypointer;

  struct ptr
    {
    Entry *p;
    // Automatic conversion to pointer for (ptr==NULL) tests
    operator Entrypointer()
      {
      return p;
      }
    // Get item at pointer
    T operator*()
      {
      return p->val;
      }
    // Get item at pointer
    T val()
      {
      return p->val;
      }
    // pointer de-reference
    T operator->()
      {
      return p->val;
      }
    // Get next pointer
    ptr next()
      {
      ptr n(p->dnext);
      return n;
      }
    // Move pointer to next
    void operator++(int)
      {
      p = p->dnext;
      }
    // Move pointer to next
    void operator++()
      {
      p = p->dnext;
      }
    // Move pointer to prev
    void operator--(int)
      {
      p = p->dprev;
      }
    // Move pointer to prev
    void operator--()
      {
      p = p->dprev;
      }
    // Get previous pointer
    ptr prev()
      {
      ptr n(p->dprev);
      return n;
      }
    ptr()
      {
      }
    ptr(Entry *e)
      {
      p = e;
      }
    };

  // Get first pointer
  ptr first()
    {
    ptr p(dfirst);
    return p;
    }

  // Get last pointer
  ptr last()
    {
    ptr p(dlast);
    return p;
    }

  // Create list
  Dlist()
    {
    dfirst = 0;
    dlast = 0;
    dlen = 0;
    last_idx = -2;
    }

  // Delete list and items in it
  ~Dlist()
    {
    ptr e, n;
    for (e = first();e;e = n)
      {
      n = e.next();
      delete e.p;
      }
    }

  // Return reference so it can be used on the left side
  T &operator[](int i)
    {
    if (i == 0)
      {
      last_idx = i;
      last_ref = dfirst;
      return last_ref->val;
      }
    else if (i == dlen - 1)
      {
      last_idx = i;
      last_ref = dlast;
      return last_ref->val;
      }
    else
      {
      last_idx = 0;
      last_ref = dfirst;
      }
    if (i > last_idx)
      {
      do
        {
        ++last_idx;
        last_ref = last_ref -> dnext;
        } while (i > last_idx);
      }
    else if (i < last_idx)
      {
      do
        {
        --last_idx;
        last_ref = last_ref -> dprev;
        } while (i < last_idx);
      }

    return last_ref -> val;
    }

  // Get pointer to nth item
  ptr nth(int i)
    {
    if (i == 0)
      {
      last_idx = i;
      last_ref = dfirst;
      return new ptr(last_ref);
      }
    else if (i == dlen - 1)
      {
      last_idx = i;
      last_ref = dlast;
      return new ptr(last_ref);
      }
    else
      {
      last_idx = 0;
      last_ref = dfirst;
      }
    if (i > last_idx)
      {
      do
        {
        ++last_idx;
        last_ref = last_ref -> dnext;
        } while (i > last_idx);
      }
    else if (i < last_idx)
      {
      do
        {
        --last_idx;
        last_ref = last_ref -> dprev;
        } while (i < last_idx);
      }

    return new ptr(last_ref);
    }

  // Search by value
  ptr search(T val)
    {
    ptr p;
    for (p = first(); p; p = p.next())
      if (p->val == val)
        break;
    return p;
    }

  // Add item to end of list (push_back)
  void add(T val)
    {
    Entry *e = new Entry;
    e->val = val;
    ++dlen;
    if (dfirst)
      {
      e->dprev = dlast;
      dlast->dnext = e;
      dlast = e;
      }
    else
      {
      dfirst = dlast = e;
      }
    }

  // Add item to beginning of list (push_front)
  void push(T val)
    {
    Entry *e = new Entry;
    e->val = val;
    ++dlen;
    if (dfirst)
      {
      e->dprev = dfirst;
      dfirst->dprev = e;
      dfirst = e;
      }
    else
      {
      dfirst = dlast = e;
      }
    }

  // Insert item before item at pointer
  void insert_before(ptr p, T val)
    {
    Entry *q = p->e;
    Entry *e = new Entry;
    e->val = val;
    ++dlen;
    e->dprev = q->dprev;
    e->dnext = q;
    if (q->dprev)
      q->dprev->dnext = e;
    else
      dfirst = e;
    q->dprev = e;
    }

  // Insert item after item at pointer
  void insert_after(ptr p, T val)
    {
    Entry *q = p->e;
    Entry *e = new Entry;
    e->val = val;
    ++dlen;
    e->dprev = q;
    e->dnext = q->dnext;
    if (q->dnext)
      q->dnext->dprev = e;
    else
      dlast = e;
    q->dnext = e;
    }

  // Delete item at pointer
  T del(ptr p)
    {
    Entry *x = p.p;
    T val = x->val;
    last_idx = -2;
    --dlen;
    if (x->dnext)
      {
      x->dnext->dprev = x->dprev;
      }
    else
      {
      dlast = x->dprev;
      }
    if (x->dprev)
      {
      x->dprev->dnext = x->dnext;
      }
    else
      {
      dfirst = x->dnext;
      }
    p.p = NULL;
    delete x;
    return val;
    }

  // Remove item from beginning of list (pop_front)
  T pop()
    {
    ptr p = first();
    if (p)
      return del(p);
    else
      return 0;
    }

  // Remove item from end of list (pop_back)
  T rmv()
    {
    ptr p = last();
    if (p)
      return del(p);
    else
      return 0;
    }
  };
