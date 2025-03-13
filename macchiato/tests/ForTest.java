package tests;

import builders.BlockBuilder;
import builders.ForBuilder;
import expressions.Literal;
import expressions.Sum;
import expressions.Variable;
import instructions.Block;

import static org.junit.jupiter.api.Assertions.*;

class ForTest {

	@org.junit.jupiter.api.Test
	void test() {
		try {
			Block block = new BlockBuilder()
				.declareVariable('a', Literal.of(1))
				.forLoop(new ForBuilder('i', Literal.of(2))
					.assign('a', Sum.of(Variable.named('a'), Literal.of(1)))
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