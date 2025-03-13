package tests;

import builders.BlockBuilder;
import builders.IfElseBuilder;
import expressions.Literal;
import expressions.Sum;
import expressions.Variable;
import instructions.Assignment;
import instructions.Block;

import static org.junit.jupiter.api.Assertions.*;

class IfTest {
	@org.junit.jupiter.api.Test
	void test() {
		try {
			Block block = new BlockBuilder()
				.declareVariable('a', Literal.of(1))
				.ifElse(new IfElseBuilder(Literal.of(5), ">=", Literal.of(4))
					.thenInstruction(new Assignment('a', Literal.of(2)))
					.build()
				)
				.ifElse(new IfElseBuilder(Literal.of(5), ">", Literal.of(6))
					.otherwiseInstruction(new Assignment('a', Sum.of(Variable.named('a'), Literal.of(1))))
					.build()
				)
				.build();

			int res = block.startProgram(false);

			assertEquals(0, res);
			assertEquals(
				3,
				block.getVariableValue('a', new Block[] {block})
			);
		}
		catch (Exception e) {
			System.out.println("Exception msg: " + e.getMessage());
			assert false;
		}
	}
}