{
  let foo = "closure"
  fn f {
    {
      print foo // [expect] closure
      let foo = "shadow"
      print foo // [expect] shadow
    }
    print foo // [expect] closure
  }
  f()
}
