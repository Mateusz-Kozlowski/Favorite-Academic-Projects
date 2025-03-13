package tests;

import expressions.Literal;
import expressions.Modulo;
import instructions.Block;

import static org.junit.jupiter.api.Assertions.assertEquals;

class ModuloTest {
	@org.junit.jupiter.api.Test
	void test() {
		try {
			assertEquals(
				1,
				Modulo.of(Literal.of(1), Literal.of(6)).evaluate(new Block[] {})
			);
			assertEquals(
				0,
				Modulo.of(Literal.of(0), Literal.of(6)).evaluate(new Block[] {})
			);
			assertEquals(
				0,
				Modulo.of(Literal.of(6), Literal.of(6)).evaluate(new Block[] {})
			);
			assertEquals(
				0,
				Modulo.of(Literal.of(18), Literal.of(6)).evaluate(new Block[] {})
			);
			assertEquals(
				2,
				Modulo.of(Literal.of(8), Literal.of(6)).evaluate(new Block[] {})
			);
		}
		catch (Exception e) {
			System.out.println("Exception msg: " + e.getMessage());
			assert false;
		}
	}
}