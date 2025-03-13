package tests;

import builders.BlockBuilder;
import expressions.Literal;
import expressions.Sum;
import expressions.Variable;
import instructions.Block;

import java.util.List;

import static org.junit.jupiter.api.Assertions.*;

class ProcedureCallTest {

	@org.junit.jupiter.api.Test
	void test() {
		try {
			Block block = new BlockBuilder()
				.declareVariable('c', Literal.of(3))
				.declareProcedure("foo1", List.of('b', 'a'), new BlockBuilder()
					.assign('c', Sum.of(Variable.named('a'), Variable.named('b')))
					.build()
				)
				.invoke("foo1", List.of(Literal.of(2), Variable.named('c')))
				.build();

			int res = block.startProgram(false);

			assertEquals(res, 0);
			assertEquals(5, block.getVariableValue('c', new Block[] {block}));
		}
		catch (Exception e) {
			System.out.println("Exception msg: " + e.getMessage());
			assert false;
		}
	}
}
