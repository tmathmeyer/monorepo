
#include <stdio.h>
#include "base/bind/bind.h"

int x(int a, int b, int c, std::unique_ptr<int> d) {
  printf("a = %i, b = %i, c = %i\n", a, b, c + *d);
  return a + b + c;
}

class Foo {
 public:
  Foo(int x) { x_ = x; }
  ~Foo() {}

  void Blah(int y) { printf("x+y = %i\n", y + x_); }

  int Z() { return x_; }

 private:
  int x_;
};

int main() {
  auto X = base::BindOnce(&x, 1, 2, 3, std::make_unique<int>(77));
  std::move(X).Run();

  auto FooCB = base::BindOnce(&Foo::Blah);
  std::move(FooCB).Run(new Foo(1), 7);

  Foo x(77);
  base::RepeatingCallback<int()> ZCB = base::BindRepeating(&Foo::Z, &x);
  int xx = ZCB.Run();
  printf("%i\n", xx);
}