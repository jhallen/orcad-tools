// Hash table template 
// Copyright (C) 2008 Joseph H. Allen
// See file COPYING for license.

template<class T>
class Hash
  {
  public:

  // Entries are in doubly-linked hash buckets, but also
  // in an overall doubly-linked list to remember total order
  // of insertion.
  struct Entry
    {
    Entry *next;
    Entry *prev;
    string name;
    T val;
    int hv;
    Entry *dnext;
    Entry *dprev;
    Entry()
      {
      next = this;
      prev = this;
      dnext = 0;
      dprev = 0;
      }
    };
  Entry *table;
  int dlen;
  int lastidx;
  Entry *dfirst;
  Entry *dlast;

  // Return number of entries in hash table
  int len()
    {
    return dlen;
    }

  // Return size of hash table
  int size()
    {
    return lastidx + 1;
    }

  // Remember last array reference
  Entry *last_ref;
  int last_idx;

  typedef Entry *Entrypointer;

  struct ptr
    {
    // Automatic conversion to pointer for (ptr==NULL) test
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
    // Get hash key
    string key()
      {
      return p->name;
      }
    // Get hash value
    int hval()
      {
      return p->hv;
      }
    void operator++(int)
      {
      p = p->dnext;
      }
    void operator++()
      {
      p = p->dnext;
      }
    void operator--(int)
      {
      p = p->dprev;
      }
    void operator--()
      {
      p = p->dprev;
      }
    // Get next pointer
    ptr next()
      {
      ptr n(p->dnext);
      return n;
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
    Entry *p;
    };

  // Return pointer to first entry
  ptr first()
    {
    ptr p(dfirst);
    return p;
    }

  // Return pointer to last entry
  ptr last()
    {
    ptr p(dlast);
    return p;
    }

  Hash()
    {
    dlen = 0;
    lastidx = 15;
    dfirst = 0;
    dlast = 0;
    last_idx = -2;
    table = new Entry[lastidx + 1];
    }

  ~Hash()
    {
    int x;
    Entry *e, *n;
    for(x = 0; x <= lastidx; ++x)
      for(e = table[x].next;e != table + x;e = n)
        {
        n = e->next;
        delete e;
        }
    delete[] table;
    }

  void enlarge(void)
    {
    int x;
    Entry *e, *n;
    int newlast = (lastidx + 1) * 2 - 1;
    Entry *newtable = new Entry[newlast + 1];
    for(x = 0;x <= lastidx; ++x)
      for(e = table[x].next; e != table + x;e = n)
        {
        Entry *f = newtable + (e->hv & newlast);
        n = e->next;
        e->next = f->next;
        e->prev = f;
        f->next->prev = e;
        f->next = e;
        }
    delete[] table;
    table = newtable;
    lastidx = newlast;
    }

  // Compute hash value from string
  int hval(string s)
    {
    int accu, x;
    for(accu = x = 0; x != s.length(); ++x) accu = (accu << 4) ^ s[x] ^ (accu >> 28);
    return accu;
    }

  // Add to just the hash table
  Entry *hash_add(string name, T val)
    {
    int idx = hval(name);
    Entry *e = new Entry;
    if(++dlen == lastidx + 1) enlarge();
    Entry *f = table + (idx & lastidx);
    e->hv = idx;
    e->name = name;
    e->val = val;
    e->next = f->next;
    e->prev = f;
    f->next->prev = e;
    f->next = e;
    return e;
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

  // Return pointer to nth item
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

  // Add item to hash table and end of list
  void add(string name,T val)
    {
    Entry *e = hash_add(name, val);
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
  void push(string name, T val)
    {
    Entry *e = hash_add(name, val);
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
  void insert_before(ptr p, string name, T val)
    {
    Entry *q = p->e;
    Entry *e = hash_add(name, val);
    e->dprev = q->dprev;
    e->dnext = q;
    if (q->dprev)
      q->dprev->dnext = e;
    else
      dfirst = e;
    q->dprev = e;
    }

  // Insert item after item at pointer
  void insert_after(ptr p, string name, T val)
    {
    Entry *q = p->e;
    Entry *e = hash_add(name, val);
    e->dprev = q;
    e->dnext = q->dnext;
    if (q->dnext)
      q->dnext->dprev = e;
    else
      dlast = e;
    q->dnext = e;
    }

  // Get pointer to an existing entry
  ptr find(string name)
    {
    ptr p(NULL);
    int idx = hval(name);
    Entry *e, *f;
    f = table + (idx & lastidx);
    for(e = f->next; e != f; e = e->next)
      if(e->name == name)
        {
        p.p = e;
        break;
        }
    return p;
    }

  // Get value associated with name
  T get(string name)
    {
    int idx = hval(name);
    Entry *e, *f;
    f = table + (idx & lastidx);
    for(e = f->next; e != f; e = e->next)
      if(e->name == name) return e->val;
    return 0;
    }

  // Look like an array with string index
  T &operator[](string name)
    {
    int idx = hval(name);
    Entry *e, *f;

    // Already exists?
    f = table + (idx & lastidx);
    for (e = f->next; e != f; e = e->next)
      if(e->name == name) return e->val;

    // Add it
    e=new Entry;
    if(++dlen == lastidx + 1) enlarge();
    f = table + (idx & lastidx);
    e->hv = idx;
    e->next = f->next;
    e->prev = f;
    f->next->prev = e;
    f->next = e;
    e->name = name;

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

    return e->val;
    }

  // Find entry with given name and delete it
  T del(string name)
    {
    int idx = hval(name);
    Entry *e, *f;
    last_idx = -2;
    f = table + (idx & lastidx);
    for(e = f->next;e != f;e = e->next)
      if(e->name == name)
        {
        T r=e->val;
        e->next->prev = e->prev;
        e->prev->next = e->next;
        if (e->dnext)
          {
          e->dnext->dprev = e->dprev;
          }
        else
          {
          dlast = e->dprev;
          }
        if (e->dprev)
          {
          e->dprev->dnext = e->dnext;
          }
        else
          {
          dfirst = e->dnext;
          }
        delete e;
        --dlen;
        return r;
        }
    return 0;
    }

  // Delete entry at pointer
  T del(ptr p)
    {
    Entry *e = p.p;
    T r = e->val;
    int idx = e->hv;
    last_idx = -2;
    e->next->prev = e->prev;
    e->prev->next = e->next;
    if (e->dnext)
      {
      e->dnext->dprev = e->dprev;
      }
    else
      {
      dlast = e->dprev;
      }
    if (e->dprev)
      {
      e->dprev->dnext = e->dnext;
      }
    else
      {
      dfirst = e->dnext;
      }
    delete e;
    --dlen;
    return r;
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
